/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/Texture3DOutputInterface.h>
#include <LumoBackend/Interfaces/ShaderUpdateInterface.h>

class ShapeModule;
class ShapeNode :
	public Texture3DOutputInterface,
	public ShaderUpdateInterface,
	public BaseNode
{
public:
	static std::shared_ptr<ShapeNode> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
	std::shared_ptr<ShapeModule> m_ShapeModulePtr = nullptr;

public:
	ShapeNode();
	~ShapeNode() override;

	// Init / Unit
	bool Init(GaiApi::VulkanCoreWeak vVulkanCore) override;

	// Execute Task
	bool ExecuteAllTime(const uint32_t & vCurrentFrame, vk::CommandBuffer * vCmd = nullptr, BaseNodeState * vBaseNodeState = nullptr) override;

	// Draw Widgets
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;

	// Resize
	void NeedResizeByResizeEvent(ct::ivec2 * vNewSize, const uint32_t * vCountColorBuffers) override;


	// Interfaces Getters
	vk::DescriptorImageInfo* GetDescriptorImage3DInfo(const uint32_t& vBindingPoint, ct::fvec3* vOutSize = nullptr) override;


	// Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

	// Shader Update
	void UpdateShaders(const std::set<std::string>& vFiles) override;

};
