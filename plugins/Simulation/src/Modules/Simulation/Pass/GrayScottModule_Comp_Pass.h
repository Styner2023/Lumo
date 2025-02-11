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

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/ShaderPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>

class GrayScottModule_Comp_Pass : public ShaderPass,

                                  public Texture2DInputInterface<1U>,
                                  public Texture2DOutputInterface {
private:
    // config name, feed, kill
    std::vector<std::string> m_GrayScottConfigNames;
    std::vector<ct::fvec4> m_GrayScottConfigs;  // diff x, diff,y, feed, kill
    int32_t m_SelectedGrayScottConfig = 0;      // Custom

    VulkanBufferObjectPtr m_UBOCompPtr = nullptr;
    vk::DescriptorBufferInfo m_UBOComp_BufferInfos = {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    struct UBOComp {
        alignas(4) float mouse_radius = 10.0f;
        alignas(4) float mouse_inversion = 0.0f;
        alignas(4) float reset_substances = 0.0f;
        alignas(4) float grayscott_diffusion_u = 1.0;
        alignas(4) float grayscott_diffusion_v = 1.0;
        alignas(4) float grayscott_feed = 0.026f;
        alignas(4) float grayscott_kill = 0.051f;
        alignas(8) ct::ivec2 image_size = 0;
    } m_UBOComp;

public:
    GrayScottModule_Comp_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~GrayScottModule_Comp_Pass() override;

    void ActionBeforeInit() override;
    void WasJustResized() override;
    void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas = nullptr) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    void ClearGrayScottConfigs();
    void AddGrayScottConfig(
        const std::string& vConfigName, const float& vDiffXValue, const float& vDiffYValue, const float& vFeedValue, const float& vKillValue);

protected:
    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};