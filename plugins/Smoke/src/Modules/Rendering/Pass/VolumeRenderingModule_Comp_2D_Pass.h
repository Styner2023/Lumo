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

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/ShaderPass.h>
#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/Texture3DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class VolumeRenderingModule_Comp_2D_Pass :
	public ShaderPass,
	public Texture3DInputInterface<1>,
	public Texture2DOutputInterface,
	public NodeInterface
{
public:
	static std::shared_ptr<VolumeRenderingModule_Comp_2D_Pass> Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore);

private:
	struct UBO_Comp {
		alignas(4) float u_Name = 0.0f;
	} m_UBO_Comp;
	VulkanBufferObjectPtr m_UBO_Comp_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };


public:
	VolumeRenderingModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
	~VolumeRenderingModule_Comp_2D_Pass() override;

	void ActionBeforeInit() override;
	void WasJustResized() override;

	void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

	// Interfaces Setters
	void SetTexture3D(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImage3DInfo, ct::fvec3* vTextureSize = nullptr) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;


	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};