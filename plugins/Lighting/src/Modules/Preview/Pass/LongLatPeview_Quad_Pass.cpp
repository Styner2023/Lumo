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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "LongLatPeview_Quad_Pass.h"

#include <cinttypes>
#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

LongLatPeview_Quad_Pass::LongLatPeview_Quad_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL) {
    SetRenderDocDebugName("Quad Pass : LongLat Peview", QUAD_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

LongLatPeview_Quad_Pass::~LongLatPeview_Quad_Pass() {
    Unit();
}

void LongLatPeview_Quad_Pass::ActionBeforeInit() {
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool LongLatPeview_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    ZoneScoped;

    bool change = false;

    // change |= DrawResizeWidget();

    if (change) {
        // NeedNewUBOUpload();
        // NeedNewSBOUpload();
    }

    return change;
}

bool LongLatPeview_Quad_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool LongLatPeview_Quad_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void LongLatPeview_Quad_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                if (vTextureSize) {
                    m_ImageInfosSize[vBindingPoint] = *vTextureSize;
                }

                m_ImageInfos[vBindingPoint] = *vImageInfo;

                if (vBindingPoint == 0U) {
                    m_UBOFrag.u_use_longlat_map = 1.0f;
                    NeedNewUBOUpload();
                }

            } else {
                if (vBindingPoint == 0U) {
                    m_UBOFrag.u_use_longlat_map = 0.0f;
                    NeedNewUBOUpload();
                }

                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* LongLatPeview_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;
    if (m_FrameBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_FrameBufferPtr->GetOutputSize();
        }

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LongLatPeview_Quad_Pass::WasJustResized() {
    ZoneScoped;
}

bool LongLatPeview_Quad_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOFrag), "LongLatPeview_Quad_Pass");
    m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBOFragPtr) {
        m_UBO_Frag_BufferInfos.buffer = m_UBOFragPtr->buffer;
        m_UBO_Frag_BufferInfos.range = sizeof(UBOFrag);
        m_UBO_Frag_BufferInfos.offset = 0;
    }

    NeedNewUBOUpload();

    return true;
}

void LongLatPeview_Quad_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void LongLatPeview_Quad_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOFragPtr.reset();
    m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool LongLatPeview_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    return res;
}

bool LongLatPeview_Quad_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorImage(1U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);   // longlat
    res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBO_Frag_BufferInfos);  // ubo frag
    return res;
}
std::string LongLatPeview_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "LongLatPeview_Quad_Pass_Vertex";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;
layout(location = 0) out vec3 v_rd;
)" + CommonSystem::Instance()->GetBufferObjectStructureHeader(0U) +
           u8R"(
vec3 getRayDirection(vec2 uv)
{
	uv = uv * 2.0 - 1.0;
	vec4 ray_clip = vec4(uv.x, uv.y, -1.0, 0.0);
	vec4 ray_eye = inverse(proj) * ray_clip;
	vec3 rd = normalize(vec3(ray_eye.x, ray_eye.y, -1.0));
	rd *= mat3(view * model);
	return rd;
}

void main() 
{
	v_rd = getRayDirection(vertUv);
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
)";
}

std::string LongLatPeview_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "LongLatPeview_Quad_Pass_Fragment";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec3 v_rd;

layout(binding = 1) uniform sampler2D longlat_map_sampler;

layout (std140, binding = 2) uniform UBO_Frag 
{ 
	float u_use_longlat_map;
};

void main() 
{
	fragColor = vec4(0);

	if (u_use_longlat_map > 0.5)
	{
		const float _pi = radians(180.0);
		const float _2pi = radians(360.0);

		vec3 rd = normalize(v_rd);
		float theta = atan(rd.x,rd.z);
		float phi =  asin(rd.y);
		vec2 uv = 0.5 + vec2(-theta / _2pi, -phi / _pi);
		fragColor = texture(longlat_map_sampler, uv);
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string LongLatPeview_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += ShaderPass::getXml(vOffset, vUserDatas);
    // str += vOffset + "<mouse_radius>" + ct::toStr(m_UBOComp.mouse_radius) + "</mouse_radius>\n";

    return str;
}

bool LongLatPeview_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    ZoneScoped;

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    ShaderPass::setFromXml(vElem, vParent, vUserDatas);

    if (strParentName == "longlat_preview_module") {
        // if (strName == "mouse_radius")
        //	m_UBOComp.mouse_radius = ct::fvariant(strValue).GetF();
    }

    return true;
}
