/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "ModelSSSNode.h"
#include <Modules/Lighting/ModelSSSModule.h>
#include <Interfaces/LightOutputInterface.h>

std::shared_ptr<ModelSSSNode> ModelSSSNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ModelSSSNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ModelSSSNode::ModelSSSNode() : BaseNode()
{
	m_NodeTypeString = "MODEL_SSS";
}

ModelSSSNode::~ModelSSSNode()
{
	Unit();
}

bool ModelSSSNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Model SSS";

	NodeSlot slot;

	slot.slotType = NodeSlotTypeEnum::LIGHT;
	slot.name = "Light";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Position";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::DEPTH;
	slot.name = "SSS Map";
	slot.descriptorBinding = 1U;
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;
	m_ModelSSSModulePtr = ModelSSSModule::Create(vVulkanCorePtr);
	if (m_ModelSSSModulePtr)
	{
		res = true;
	}

	return res;
}

void ModelSSSNode::Unit()
{
	m_ModelSSSModulePtr.reset();
}

bool ModelSSSNode::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	// for update input texture buffer infos => avoid vk crash
	UpdateInputDescriptorImageInfos(m_Inputs);

	if (m_ModelSSSModulePtr)
	{
		return m_ModelSSSModulePtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool ModelSSSNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_ModelSSSModulePtr)
	{
		return m_ModelSSSModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ModelSSSNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_ModelSSSModulePtr)
	{
		m_ModelSSSModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

void ModelSSSNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_ModelSSSModulePtr)
	{
		m_ModelSSSModulePtr->SetLightGroup(vSceneLightGroup);
	}
}

void ModelSSSNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	if (m_ModelSSSModulePtr)
	{
		m_ModelSSSModulePtr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* ModelSSSNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_ModelSSSModulePtr)
	{
		return m_ModelSSSModulePtr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void ModelSSSNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ModelSSSModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					SetTexture(startSlotPtr->descriptorBinding, otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding));
				}
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT)
			{
				auto otherLightGroupNodePtr = dynamic_pointer_cast<LightOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherLightGroupNodePtr)
				{
					SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void ModelSSSNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ModelSSSModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				SetTexture(startSlotPtr->descriptorBinding, nullptr);
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT)
			{
				SetLightGroup();
			}
		}
	}
}

void ModelSSSNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<TextureOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						SetTexture(receiverSlotPtr->descriptorBinding, otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding));
					}
				}
			}
		}

		//todo emit notification
		break;
	}
	case NotifyEvent::LightUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<LightOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetLightGroup(otherNodePtr->GetLightGroup());
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

void ModelSSSNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_ModelSSSModulePtr)
	{
		m_ModelSSSModulePtr->NeedResize(vNewSize, vCountColorBuffer);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffer);
}

void ModelSSSNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_ModelSSSModulePtr)
	{
		return m_ModelSSSModulePtr->UpdateShaders(vFiles);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelSSSNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string res;

	if (!m_ChildNodes.empty())
	{
		res += BaseNode::getXml(vOffset, vUserDatas);
	}
	else
	{
		res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n",
			name.c_str(),
			m_NodeTypeString.c_str(),
			ct::fvec2(pos.x, pos.y).string().c_str(),
			(uint32_t)nodeID.Get());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_ModelSSSModulePtr)
		{
			res += m_ModelSSSModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ModelSSSNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	BaseNode::setFromXml(vElem, vParent, vUserDatas);

	if (m_ModelSSSModulePtr)
	{
		m_ModelSSSModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}