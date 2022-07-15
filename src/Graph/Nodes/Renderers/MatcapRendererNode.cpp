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

#include "MatcapRendererNode.h"
#include <Modules/Renderers/MatcapRenderer.h>
#include <Interfaces/ModelOutputInterface.h>

std::shared_ptr<MatcapRendererNode> MatcapRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<MatcapRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

MatcapRendererNode::MatcapRendererNode() : BaseNode()
{
	m_NodeTypeString = "MATCAP_RENDERER";
}

MatcapRendererNode::~MatcapRendererNode()
{
	Unit();
}

bool MatcapRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Matcap";

	NodeSlot slot;

	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.name = "3D Model";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "2D Texture";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;

	m_MatcapRenderer = MatcapRenderer::Create(vVulkanCorePtr);
	if (m_MatcapRenderer)
	{
		res = true;
	}

	return res;
}

void MatcapRendererNode::Unit()
{
	m_MatcapRenderer.reset();
}

bool MatcapRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_MatcapRenderer)
	{
		return m_MatcapRenderer->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool MatcapRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_MatcapRenderer)
	{
		return m_MatcapRenderer->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void MatcapRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_MatcapRenderer)
	{
		m_MatcapRenderer->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void MatcapRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used(%s)\nCell(%i, %i)"/*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/,
				(used ? "true" : "false"), cell.x, cell.y/*, pos.x, pos.y, size.x, size.y*/);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

void MatcapRendererNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_MatcapRenderer)
	{
		m_MatcapRenderer->NeedResize(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffers);
}

void MatcapRendererNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_MatcapRenderer)
	{
		m_MatcapRenderer->SetModel(vSceneModel);
	}
}

void MatcapRendererNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	if (m_MatcapRenderer)
	{
		m_MatcapRenderer->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* MatcapRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_MatcapRenderer)
	{
		return m_MatcapRenderer->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void MatcapRendererNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_MatcapRenderer)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::MESH)
			{
				auto otherModelNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherModelNodePtr)
				{
					SetModel(otherModelNodePtr->GetModel());
				}
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					SetTexture(startSlotPtr->descriptorBinding, otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding));
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void MatcapRendererNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_MatcapRenderer)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::MESH)
			{
				SetModel();
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				SetTexture(startSlotPtr->descriptorBinding, nullptr);
			}
		}
	}
}

void MatcapRendererNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<ModelOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetModel(otherNodePtr->GetModel());
				}
			}
		}
		break;
	}
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
		break;
	}
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MatcapRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_MatcapRenderer)
		{
			res += m_MatcapRenderer->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool MatcapRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_MatcapRenderer)
	{
		m_MatcapRenderer->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}