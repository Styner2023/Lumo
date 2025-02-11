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
#include <LumoBackend/Base/EffectPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class BloomModule_Comp_2D_Pass : public EffectPass<1U>, public NodeInterface {
public:
    static std::shared_ptr<BloomModule_Comp_2D_Pass> Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore);

private:
    struct UBOComp {
        alignas(16) ct::fvec3 u_high_freq_threshold = 0.8f;
        alignas(4) uint32_t u_blur_radius = 4;  // default is 4
        alignas(4) float u_exposure = 1.0f;
        alignas(4) float u_gamma_correction = 2.2f;  // linear to srgb correction
        alignas(4) float u_enabled = 1.0f;
    } m_UBOComp;
    VulkanBufferObjectPtr m_UBOCompPtr = nullptr;
    vk::DescriptorBufferInfo m_DescriptorBufferInfo_Comp;

    bool m_GaussianSigmAuto = true;
    float m_GaussianSigma = 1.0f;
    std::vector<float> m_GaussianWeights;
    VulkanBufferObjectPtr m_SBO_GaussianWeights = nullptr;
    vk::DescriptorBufferInfo m_SBO_GaussianWeightsBufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

public:
    BloomModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~BloomModule_Comp_2D_Pass() override;
    void ActionBeforeInit() override;
    void ActionBeforeCompilation() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas = nullptr) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;
    void SwapMultiPassFrontBackDescriptors() override;
    bool CanUpdateDescriptors() override;
    void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void AfterNodeXmlLoading() override;

protected:
    void ReComputeGaussianBlurWeights();

    void Compute_High_Freq_Thresholding(vk::CommandBuffer* vCmdBufferPtr);
    void Compute_Horizontal_Blur(vk::CommandBuffer* vCmdBufferPtr);
    void Compute_Vertical_Blur(vk::CommandBuffer* vCmdBufferPtr);
    void Compute_Gamma_Correction(vk::CommandBuffer* vCmdBufferPtr);

    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool CreateSBO() override;
    void UploadSBO() override;
    void DestroySBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    bool CreateComputePipeline() override;

    std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};