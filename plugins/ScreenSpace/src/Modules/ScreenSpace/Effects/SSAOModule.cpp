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

#include "SSAOModule.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/ScreenSpace/Effects/Pass/SSAOModule_Comp_2D_Pass.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<SSAOModule> SSAOModule::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<SSAOModule>(vVulkanCore);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SSAOModule::SSAOModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

SSAOModule::~SSAOModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SSAOModule::Init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;
    if (BaseRenderer::InitCompute2D(map_size)) {
        m_SSAOModule_Comp_2D_Pass_Ptr = SSAOModule_Comp_2D_Pass::Create(map_size, m_VulkanCore);
        if (m_SSAOModule_Comp_2D_Pass_Ptr) {
            // by default but can be changed via widget
            m_SSAOModule_Comp_2D_Pass_Ptr->AllowResizeOnResizeEvents(true);
            m_SSAOModule_Comp_2D_Pass_Ptr->AllowResizeByHandOrByInputs(false);
            AddGenericPass(m_SSAOModule_Comp_2D_Pass_Ptr);
            m_Loaded = true;
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SSAOModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseRenderer::Render("SSAO", vCmd);

    return true;
}

bool SSAOModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame && m_SSAOModule_Comp_2D_Pass_Ptr) {
        return m_SSAOModule_Comp_2D_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool SSAOModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool SSAOModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

void SSAOModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void SSAOModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (m_SSAOModule_Comp_2D_Pass_Ptr) {
            m_SSAOModule_Comp_2D_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
        }
    }
}

vk::DescriptorImageInfo* SSAOModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;

    if (m_SSAOModule_Comp_2D_Pass_Ptr) {
        return m_SSAOModule_Comp_2D_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SSAOModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<ssao_module>\n";

    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

    if (m_SSAOModule_Comp_2D_Pass_Ptr) {
        str += m_SSAOModule_Comp_2D_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
    }

    str += vOffset + "</ssao_module>\n";

    return str;
}

bool SSAOModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "ssao_module") {
        if (strName == "can_we_render") {
            m_CanWeRender = ct::ivariant(strValue).GetB();
        }
    }

    if (m_SSAOModule_Comp_2D_Pass_Ptr) {
        m_SSAOModule_Comp_2D_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void SSAOModule::AfterNodeXmlLoading() {
    if (m_SSAOModule_Comp_2D_Pass_Ptr) {
        m_SSAOModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    }
}
