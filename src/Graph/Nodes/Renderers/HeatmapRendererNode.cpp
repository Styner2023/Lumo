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

#include "HeatmapRendererNode.h"
#include <Modules/Renderers/HeatmapRenderer.h>
#include <Interfaces/ModelOutputInterface.h>

std::shared_ptr<HeatmapRendererNode> HeatmapRendererNode::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<HeatmapRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}
	return res;
}

HeatmapRendererNode::HeatmapRendererNode() : BaseNode()
{
	m_NodeType = NodeTypeEnum::HEATMAP_RENDERER;
}

HeatmapRendererNode::~HeatmapRendererNode()
{
	Unit();
}

bool HeatmapRendererNode::Init(vkApi::VulkanCore* vVulkanCore)
{
	name = "Heatmap";

	NodeSlot slot;
	
	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.name = "3D Model";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	AddOutput(slot, true, true);

	bool res = false;

	m_HeatmapRenderer = HeatmapRenderer::Create(vVulkanCore);
	if (m_HeatmapRenderer)
	{
		res = true;
	}

	return res;
}

void HeatmapRendererNode::Unit()
{
	m_HeatmapRenderer.reset();
}

bool HeatmapRendererNode::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	if (m_HeatmapRenderer)
	{
		return m_HeatmapRenderer->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool HeatmapRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_HeatmapRenderer)
	{
		return m_HeatmapRenderer->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void HeatmapRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_HeatmapRenderer)
	{
		m_HeatmapRenderer->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void HeatmapRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState)
{
	if (vCanvasState && vCanvasState->debug_mode)
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

void HeatmapRendererNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_HeatmapRenderer)
	{
		m_HeatmapRenderer->NeedResize(vNewSize, vCountColorBuffer);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffer);
}

void HeatmapRendererNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_HeatmapRenderer)
	{
		m_HeatmapRenderer->SetModel(vSceneModel);
	}
}

vk::DescriptorImageInfo* HeatmapRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_HeatmapRenderer)
	{
		return m_HeatmapRenderer->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void HeatmapRendererNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_HeatmapRenderer)
	{
		if (startSlotPtr->IsAnInput())
		{
			auto otherNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
			if (otherNodePtr)
			{
				SetModel(otherNodePtr->GetModel());
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void HeatmapRendererNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_HeatmapRenderer)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::MESH)
			{
				SetModel();
			}
		}
	}
}

void HeatmapRendererNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
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
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string HeatmapRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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
			Graph::GetStringFromNodeTypeEnum(m_NodeType).c_str(),
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

		if (m_HeatmapRenderer)
		{
			res += m_HeatmapRenderer->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool HeatmapRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_HeatmapRenderer)
	{
		m_HeatmapRenderer->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}