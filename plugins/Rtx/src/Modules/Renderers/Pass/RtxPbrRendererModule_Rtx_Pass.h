/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

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
#include <Base/RtxShaderPass.h>
#include <vulkan/vulkan.hpp>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Core/vk_mem_alloc.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <Interfaces/AccelStructureInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class RtxPbrRendererModule_Rtx_Pass : public RtxShaderPass,
                                      public AccelStructureInputInterface,
                                      public LightGroupInputInterface,
                                      public Texture2DInputInterface<3u>,
                                      public Texture2DOutputInterface,
                                      public NodeInterface {
private:
    struct UBO_Chit {
        alignas(4) float u_diffuse_factor = 1.0f;
        alignas(4) float u_metallic_factor = 1.0f;
        alignas(4) float u_rugosity_factor = 1.0f;
        alignas(4) float u_ao_factor = 1.0f;
        alignas(4) float u_use_albedo_map = 0.0f;
        alignas(4) float u_use_ao_map = 0.0f;
        alignas(4) float u_use_longlat_map = 0.0f;
        alignas(4) float u_light_intensity_factor = 100.0f;
        alignas(4) float u_shadow_strength = 0.5f;
    } m_UBO_Chit;
    VulkanBufferObjectPtr m_UBO_Chit_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Chit_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

public:
    RtxPbrRendererModule_Rtx_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~RtxPbrRendererModule_Rtx_Pass() override;

    void ActionBeforeCompilation() override;
    void ActionBeforeInit() override;
    void WasJustResized() override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    // Interfaces Setters
    void SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) override;
    void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas = nullptr) override;

    // Interfaces Getters
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void AfterNodeXmlLoading() override;

protected:
    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool CanUpdateDescriptors() override;
    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    std::string GetHitPayLoadCode();
    std::string GetRayGenerationShaderCode(std::string& vOutShaderName) override;
    std::string GetRayIntersectionShaderCode(std::string& vOutShaderName) override;
    std::string GetRayMissShaderCode(std::string& vOutShaderName) override;
    std::string GetRayAnyHitShaderCode(std::string& vOutShaderName) override;
    std::string GetRayClosestHitShaderCode(std::string& vOutShaderName) override;
};