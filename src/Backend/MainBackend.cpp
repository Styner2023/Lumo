﻿#include "MainBackend.h"

#include <GLFW/glfw3.h>
#include <ctools/FileHelper.h>

#include <cstdio>     // printf, fprintf
#include <chrono>     // timer
#include <cstdlib>    // abort
#include <fstream>    // std::ifstream
#include <iostream>   // std::cout
#include <algorithm>  // std::min, std::max
#include <stdexcept>  // std::exception

#include <Backend/MainBackend.h>
#include <Plugins/PluginManager.h>
#include <Graph/Manager/NodeManager.h>
#include <Graph/Library/UserNodeLibrary.h>
#include <Scene/Manager/SceneManager.h>
#include <LayoutManager.h>

#include <ImGuiPack.h>

#include <Headers/Globals.h>
#include <Headers/LumoBuild.h>

#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Frontend/MainFrontend.h>

#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanImGuiRenderer.h>
#include <Gaia/Gui/VulkanProfiler.h>

#include <LumoBackend/Systems/SceneMerger.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <LumoBackend/Systems/FilesTrackerSystem.h>
#include <LumoBackend/Helpers/RenderDocController.h>

#include <Headers/LumoBuild.h>

#include <Panes/DebugPane.h>
#include <Panes/GraphPane.h>
#include <Panes/TuningPane.h>
#include <Panes/View2DPane.h>
#include <Panes/View3DPane.h>
#include <Panes/ConsolePane.h>
#include <Panes/SceneViewPane.h>

#include <Project/ProjectFile.h>

using namespace gaia;

// #define CREATE_IN_APP_SHADER_FILES

#define USE_RTX true
#define INITIAL_WIDTH 1700
#define INITIAL_HEIGHT 700

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

static void glfw_window_close_callback(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GL_FALSE);  // block app closing
    auto overLayPtr = MainBackend::Instance()->GetOverlay().lock();
    assert(overLayPtr != nullptr);
    overLayPtr->getFrontend()->Action_Window_CloseApp();
}

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainBackendPtr MainBackend::Create() {
    auto res = std::make_shared<MainBackend>();
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainBackend::~MainBackend() = default;

void MainBackend::run() {
    if (init()) {
        m_MainLoop();
        unit();
    }
}

bool MainBackend::init() {
#ifdef _DEBUG
    SetConsoleVisibility(true);
#else
    SetConsoleVisibility(false);
#endif
    if (m_CreateVulkanWindow()) {
        if (glfwVulkanSupported()) {
            m_InitFilesTracker();
            if (m_CreateVulkanCore()) {
                if (m_CreateImGuiOverlay()) {
                    m_InitPlugins();
                    m_InitScenes();
                    m_InitNodes();
                    m_Resize();
                    m_InitSystems();
                    m_InitPanes();
                    // if (m_CreateRenderers()) {
                    LoadConfigFile("config.xml");
                    if (!m_ProjectFileToLoad.empty()) {
                        // loadSdfShader(m_ProjectFileToLoad);
                    }
                    return true;
                    //}
                }
            }
        }
    }
    return false;
}

void MainBackend::unit() {
    SaveConfigFile("config.xml");
    m_FileDialogAssets.clear();
    m_VulkanCorePtr->getDevice().waitIdle();
    m_UnitSystems();
    m_DestroyRenderers();
    m_DestroyImGuiOverlay();
    m_UnitNodes();
    m_UnitScenes();
    m_UnitPlugins();
    m_DestroyVulkanCore();
    m_UnitFilesTracker();
    m_DestroyVulkanWindow();
}

bool MainBackend::isValid() const {
    return (m_VulkanCorePtr != nullptr &&    //
            m_VulkanWindowPtr != nullptr &&  //
            m_ImGuiOverlayPtr != nullptr);   //
}

bool MainBackend::isThereAnError() const {
    return false;
}

void MainBackend::NeedToNewProject(const std::string& vFilePathName) {
    m_NeedToNewProject = true;
    m_ProjectFileToLoad = vFilePathName;
}

void MainBackend::NeedToLoadProject(const std::string& vFilePathName) {
    m_NeedToLoadProject = true;
    m_ProjectFileToLoad = vFilePathName;
}

void MainBackend::NeedToCloseProject() {
    m_NeedToCloseProject = true;
}

bool MainBackend::SaveProject() {
    return ProjectFile::Instance()->Save();
}

void MainBackend::SaveAsProject(const std::string& vFilePathName) {
    ProjectFile::Instance()->SaveAs(vFilePathName);
}

// actions to do after rendering
void MainBackend::PostRenderingActions() {
    if (m_NeedToNewProject) {
        ProjectFile::Instance()->Clear();
        ProjectFile::Instance()->New(m_ProjectFileToLoad);
        m_VulkanWindowPtr->setAppTitle(m_ProjectFileToLoad);
        m_ProjectFileToLoad.clear();
        m_NeedToNewProject = false;
    }

    if (m_NeedToLoadProject) {
        if (ProjectFile::Instance()->LoadAs(m_ProjectFileToLoad)) {
            m_VulkanWindowPtr->setAppTitle(m_ProjectFileToLoad);
            ProjectFile::Instance()->SetProjectChange(false);
        } else {
            LogVarError("Failed to load project %s", m_ProjectFileToLoad.c_str());
        }

        m_ProjectFileToLoad.clear();
        m_NeedToLoadProject = false;
    }

    if (m_NeedToCloseProject) {
        NodeManager::Instance()->Clear();
        ProjectFile::Instance()->Clear();
        m_NeedToCloseProject = false;
    }
}

bool MainBackend::IsNeedToCloseApp() {
    return m_NeedToCloseApp;
}

void MainBackend::NeedToCloseApp(const bool& vFlag) {
    m_NeedToCloseApp = vFlag;
}

void MainBackend::CloseApp() {
    assert(m_VulkanWindowPtr != nullptr);
    m_VulkanWindowPtr->CloseWindowWhenPossible();
}

void MainBackend::setSize(const ct::ivec2& vSize) {
    m_size = vSize;
}

const ct::ivec2& MainBackend::getSize() const {
    return m_size;
}

bool MainBackend::resize(const ct::ivec2& vNewSize) {
    m_DisplaySize = vNewSize;
    bool res = true;
    /*if (m_DisplaySizeQuadRendererPtr != nullptr) {
        m_DisplaySizeQuadRendererPtr->NeedResizeByHand(&m_DisplaySize);
        res &= m_DisplaySizeQuadRendererPtr->ResizeIfNeeded();
    }*/
    return res;
}

void MainBackend::setAppTitle(const std::string& vFilePathName) {
    assert(m_VulkanWindowPtr != nullptr);
    auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
    if (ps.isOk) {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "Lumo Beta %s - Project : %s.lum", Lumo_BuildId, ps.name.c_str());
        m_VulkanWindowPtr->setAppTitle(bufTitle);
    } else {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "Lumo Beta %s", Lumo_BuildId);
        m_VulkanWindowPtr->setAppTitle(bufTitle);
    }
}

void MainBackend::setFrontend(const MainFrontendWeak& vFontend) {
    m_Frontend = vFontend;
}

ct::dvec2 MainBackend::GetMousePos() {
    if (m_VulkanWindowPtr) {
        ct::dvec2 mp;
        glfwGetCursorPos(m_VulkanWindowPtr->getWindowPtr(), &mp.x, &mp.y);
        return mp;
    }
    return -1;
}

int MainBackend::GetMouseButton(int vButton) {
    if (m_VulkanWindowPtr) {
        return glfwGetMouseButton(m_VulkanWindowPtr->getWindowPtr(), vButton);
    }
    return -1;
}

std::string MainBackend::getAppRelativeFilePathName(const std::string& vFilePathName) {
    if (!vFilePathName.empty()) {
        if (FileHelper::Instance()->IsFileExist(vFilePathName)) {
            auto file_path_name = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
            return FileHelper::Instance()->GetPathRelativeToApp(file_path_name);
        }
    }
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONSOLE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainBackend::SetConsoleVisibility(const bool& vFlag) {
    m_ConsoleVisiblity = vFlag;

    if (m_ConsoleVisiblity) {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    } else {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    }
}

void MainBackend::SwitchConsoleVisibility() {
    m_ConsoleVisiblity = !m_ConsoleVisiblity;
    SetConsoleVisibility(m_ConsoleVisiblity);
}

bool MainBackend::GetConsoleVisibility() {
    return m_ConsoleVisiblity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// RENDER ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainBackend::m_RenderOffScreen() {
    // m_DisplaySizeQuadRendererPtr->SetImageInfos(m_MergerRendererPtr->GetBackDescriptorImageInfo(0U));
}

void MainBackend::m_MainLoop() {
    auto main_window_ptr = m_VulkanWindowPtr->getWindowPtr();
    while (!glfwWindowShouldClose(main_window_ptr)) {
        ZoneScoped;

        vkProfBeginFrame("GPU Frame");

        ProjectFile::Instance()->NewFrame();

        RenderDocController::Instance()->StartCaptureIfResquested();

        // maintain active, prevent user change via imgui dialog
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;  // Disable Viewport

        glfwPollEvents();

        // to absolutly do beofre all vk rendering commands
        m_VulkanCorePtr->ResetCommandPools();

        m_Update();  // to do absolutly beofre imgui rendering

        const auto& reso = m_VulkanWindowPtr->getWindowResolution();
        m_PrepareImGui(ImRect(ImVec2(), ImVec2(reso.x, reso.y)));

        // Merged Rendering
        bool needResize = false;
        if (m_BeginRender(needResize)) {
            m_ImGuiOverlayPtr->render();  // gui rendering
            m_EndRender();
        } else if (needResize) {
            m_Resize();
        }

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste
        // this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

#ifdef USE_THUMBNAILS
        m_VulkanCorePtr->getDevice().waitIdle();
        ImGuiFileDialog::Instance()->ManageGPUThumbnails();
#endif

        RenderDocController::Instance()->EndCaptureIfResquested();

        // delete imgui nodes now
        // like that => no issue with imgui descriptors because after imgui render and before next node computing
        m_DeleteNodesIfAnys();

        // mainframe post actions
        PostRenderingActions();

        ++m_CurrentFrame;

        vkProfEndFrame;
        vkProfCollectFrame;

        // will pause the view until we move the mouse or press keys
        // glfwWaitEvents();
    }
}

bool MainBackend::m_BeginRender(bool& vNeedResize) {
    ZoneScoped;
    if (m_VulkanCorePtr->AcquireNextImage(m_VulkanWindowPtr)) {
        if (m_VulkanCorePtr->frameBegin()) {
            vkProfBeginZone(m_VulkanCorePtr->getGraphicCommandBuffer(), "ImGui", "%s", "Render");
            m_VulkanCorePtr->beginMainRenderPass();
            return true;
        }
    } else {  // maybe a resize will fix
        vNeedResize = true;
    }

    return false;
}

void MainBackend::m_EndRender() {
    ZoneScoped;
    m_VulkanCorePtr->endMainRenderPass();
    vkProfEndZone(m_VulkanCorePtr->getGraphicCommandBuffer());
    m_VulkanCorePtr->frameEnd();
    m_VulkanCorePtr->Present();
}

void MainBackend::m_PrepareImGui(ImRect vViewport) {
    ZoneScoped;

    ImGui::SetPUSHID(125);
    m_ImGuiOverlayPtr->begin();
    ImGuizmo::BeginFrame();

    FrameMark;

    m_ImGuiOverlayPtr->getFrontend()->Display(m_CurrentFrame);
    NodeManager::Instance()->DrawDialogsAndPopups(m_CurrentFrame, vViewport, ImGui::GetCurrentContext(), {});
    m_ImGuiOverlayPtr->end();
}

void MainBackend::m_Update() {
    ZoneScoped;

    m_CheckIfTheseAreSomeFileChanges();
    m_UpdateCameraAndMouse();
    m_UpdateSound();

    m_VulkanCorePtr->GetDeltaTime(m_CurrentFrame);

    CommonSystem::Instance()->UploadBufferObjectIfDirty(m_VulkanCorePtr);

    // evaluation of the graph
    NodeManager::Instance()->Execute(m_CurrentFrame);
    SceneManager::Instance()->Execute(m_CurrentFrame);

    m_RenderOffScreen();  // frame rendering
}

void MainBackend::m_IncFrame() {
    ++m_CurrentFrame;
}

void MainBackend::m_Resize() {
    auto swapChainPtr = m_VulkanCorePtr->getSwapchain().lock();
    if (swapChainPtr != nullptr) {
        const auto& extent = swapChainPtr->getRenderArea().extent;
        ct::ivec2 new_size((int32_t)extent.width, (int32_t)extent.height);
        resize(new_size);
    }
}

void MainBackend::m_AddPathToTrack(std::string vPathToTrack, bool vCreateDirectoryIfNotExist) {
    ZoneScoped;
    if (!vPathToTrack.empty()) {
        if (vCreateDirectoryIfNotExist) {
            FileHelper::Instance()->CreateDirectoryIfNotExist(vPathToTrack);
        }
        if (m_PathsToTrack.find(vPathToTrack) == m_PathsToTrack.end()) {  // non trouve
            m_PathsToTrack.emplace(vPathToTrack);
            FileHelper::Instance()->puSearchPaths.push_back(vPathToTrack);
            FilesTrackerSystem::Instance()->addWatch(vPathToTrack);
        }
    }
}

void MainBackend::m_InitFilesTracker(std::function<void(std::set<std::string>)> vChangeFunc, std::list<std::string> vPathsToTrack) {
    ZoneScoped;
    m_ChangeFunc = vChangeFunc;
    for (auto path : vPathsToTrack) {
        m_AddPathToTrack(path, true);
    }
    FilesTrackerSystem::Instance()->Changes = false;
}

void MainBackend::m_CheckIfTheseAreSomeFileChanges() {
    ZoneScoped;
    FilesTrackerSystem::Instance()->update();
    if (FilesTrackerSystem::Instance()->Changes) {
        m_ChangeFunc(FilesTrackerSystem::Instance()->files);
        FilesTrackerSystem::Instance()->files.clear();
        FilesTrackerSystem::Instance()->Changes = false;
    }
}

void MainBackend::m_UpdateFiles(const std::set<std::string>& vFiles) {
    ZoneScoped;

    std::set<std::string> updated_files;

    for (auto file : vFiles) {
        if (file.find(".vert") != std::string::npos || file.find(".frag") != std::string::npos || file.find(".tess") != std::string::npos ||
            file.find(".eval") != std::string::npos || file.find(".glsl") != std::string::npos || file.find(".geom") != std::string::npos ||
            file.find(".scen") != std::string::npos || file.find(".blue") != std::string::npos || file.find(".comp") != std::string::npos ||
            file.find(".rgen") != std::string::npos || file.find(".rint") != std::string::npos || file.find(".miss") != std::string::npos ||
            file.find(".ahit") != std::string::npos || file.find(".chit") != std::string::npos) {
            ct::replaceString(file, "\\", "/");
            ct::replaceString(file, "./", "");
            ct::replaceString(file, "//", "/");
            updated_files.emplace(file);
        }
    }

    if (!updated_files.empty()) {
        NodeManager::Instance()->UpdateShaders(updated_files);
    }
}

///////////////////////////////////////////////////////
//// INPUTS ///////////////////////////////////////////
///////////////////////////////////////////////////////

void MainBackend::UpdateMouseDatas(bool vCanUseMouse) {
    for (auto mb = 0; mb < 3; mb++) {
        const auto imd = (GetMouseButton(mb) == 1);
        m_MouseInterface.buttonDownLastFrame[mb] = m_MouseInterface.buttonDown[mb];
        m_MouseInterface.buttonDown[mb] = imd && vCanUseMouse;
    }

    m_MouseInterface.canUpdateMouse =
        m_BackendDatas.canWeTuneMouse && (m_MouseInterface.buttonDown[0] || m_MouseInterface.buttonDown[1] || m_MouseInterface.buttonDown[2]);

    if (vCanUseMouse) {
        // m_MouseInterface pos is needed for camera and mouse
        // is will be needed to clarify that.
        // normally the mouse interface is only for mouse
        const auto p = GetMousePos();
        m_MouseInterface.px = (float)p.x;
        m_MouseInterface.py = (float)p.y;
    }
}

void MainBackend::m_UpdateCameraAndMouse() {
    ZoneScoped;

    // update mesher camera // camera of renderpack
    /*GamePadSystem::Instance()->Update();
    if (GamePadSystem::Instance()->WasChanged()) {
        CommonSystem::Instance()->m_NeedCamChange = true;
        GamePadSystem::Instance()->ResetChange();
    }*/

    // permet d'eviter de creer un gouffre entre normalizedMousePos et puLastNormalizedMousePos
    if (!ImGuizmo::IsUsing()) {
        if (m_DisplayQuality >= 0.0f) {
            const auto mousePos = -ct::fvec2(m_MouseInterface.px, m_MouseInterface.py) / m_DisplayQuality;

            if (CommonSystem::Instance()->m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE) {
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                    CommonSystem::Instance()->IncFlyingPosition(CommonSystem::Instance()->m_CameraSettings.m_SpeedFactor);
                } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                    CommonSystem::Instance()->IncFlyingPosition(-CommonSystem::Instance()->m_CameraSettings.m_SpeedFactor);
                }
            }

            if (m_MouseInterface.buttonDown[0]) {  // left mouse rotate
                if (m_BackendDatas.canWeTuneCamera || m_BackendDatas.canWeTuneMouse) {
                    m_NormalizedMousePos.x = mousePos.x / m_DisplaySize.x;
                    m_NormalizedMousePos.y = mousePos.y / m_DisplaySize.y;
                    if (!m_MouseDrag) {
                        m_LastNormalizedMousePos = m_NormalizedMousePos;
                    }
                    m_MouseDrag = true;
                }
                auto diff = m_LastNormalizedMousePos - m_NormalizedMousePos;  // inversed of have good rotation
                if (m_BackendDatas.canWeTuneCamera) {
                    CommonSystem::Instance()->IncRotateXYZ(ct::fvec3(diff * 6.28318f, 0.0f));
                }
                if (m_BackendDatas.canWeTuneCamera || m_BackendDatas.canWeTuneMouse) {
                    NeedRefresh(!diff.emptyAND());
                    m_LastNormalizedMousePos = m_NormalizedMousePos;
                }
            } else if (m_MouseInterface.buttonDown[1]) {  // right mouse zoom y and rotate x
                if (m_BackendDatas.canWeTuneCamera || m_BackendDatas.canWeTuneMouse) {
                    m_NormalizedMousePos.x = mousePos.x / m_DisplaySize.x;
                    m_NormalizedMousePos.y = mousePos.y / m_DisplaySize.y;
                    if (!m_MouseDrag) {
                        m_LastNormalizedMousePos = m_NormalizedMousePos;
                    }
                    m_MouseDrag = true;
                }
                auto diff = m_LastNormalizedMousePos - m_NormalizedMousePos;  // inversed of have good rotation
                if (m_BackendDatas.canWeTuneCamera) {
                    CommonSystem::Instance()->IncZoom(diff.y * 50.0f);
                    CommonSystem::Instance()->IncRotateXYZ(ct::fvec3(0.0f, 0.0f, diff.x * 6.28318f));
                }
                if (m_BackendDatas.canWeTuneCamera || m_BackendDatas.canWeTuneMouse) {
                    NeedRefresh(!diff.emptyAND());
                    m_LastNormalizedMousePos = m_NormalizedMousePos;
                }
            } else if (m_MouseInterface.buttonDown[2]) {  // middle mouse, translate
                if (m_BackendDatas.canWeTuneCamera || m_BackendDatas.canWeTuneMouse) {
                    m_NormalizedMousePos.x = mousePos.x / m_DisplaySize.x;
                    m_NormalizedMousePos.y = mousePos.y / m_DisplaySize.y;
                    if (!m_MouseDrag) {
                        m_LastNormalizedMousePos = m_NormalizedMousePos;
                    }
                    m_MouseDrag = true;
                }
                auto diff = m_LastNormalizedMousePos - m_NormalizedMousePos;  // inversed of have good rotation
                if (m_BackendDatas.canWeTuneCamera) {
                    CommonSystem::Instance()->IncTranslateXY(diff * 10.0f);
                }
                if (m_BackendDatas.canWeTuneCamera || m_BackendDatas.canWeTuneMouse) {
                    NeedRefresh(!diff.emptyAND());
                    m_LastNormalizedMousePos = m_NormalizedMousePos;
                }
            } else {
                m_MouseDrag = false;
            }
        }
    }

    m_UpdateCamera();
}

void MainBackend::NeedRefresh(const bool& vFlag) {
    m_NeedRefresh |= vFlag;
}

void MainBackend::m_UpdateMouse() {
}

void MainBackend::m_UpdateCamera(const bool& vForce) {
    if (vForce) {
        CommonSystem::Instance()->NeedCamChange();
    }
    View3DPane::Instance()->UpdateCameraIfNeeded();
    SceneViewPane::Instance()->UpdateCameraIfNeeded();
}

void MainBackend::m_UpdateSound() {
    // SoundSystem::Instance()->Update();
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainBackend::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += m_ImGuiOverlayPtr->getFrontend()->getXml(vOffset, vUserDatas);
    str += vOffset + "<project>" + ProjectFile::Instance()->GetProjectFilepathName() + "</project>\n";
    str += vOffset + "<can_we_tune_mouse>" + (m_BackendDatas.canWeTuneMouse ? "true" : "false") + "</can_we_tune_mouse>\n";
    str += vOffset + "<can_we_tune_gizmo>" + (m_BackendDatas.canWeTuneGizmo ? "true" : "false") + "</can_we_tune_gizmo>\n";
    str += vOffset + "<can_we_show_grid>" + (m_BackendDatas.canWeShowGrid ? "true" : "false") + "</can_we_show_grid>\n";

    return str;
}

bool MainBackend::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != 0)
        strParentName = vParent->Value();

    m_ImGuiOverlayPtr->getFrontend()->setFromXml(vElem, vParent, vUserDatas);

    if (strName == "project") {
        NeedToLoadProject(strValue);
    } else if (strName == "can_we_tune_mouse") {
        m_BackendDatas.canWeTuneMouse = ct::ivariant(strValue).GetB();
    } else if (strName == "can_we_tune_camera") {
        m_BackendDatas.canWeTuneCamera = ct::ivariant(strValue).GetB();
    } else if (strName == "can_we_tune_gizmo") {
        m_BackendDatas.canWeTuneGizmo = ct::ivariant(strValue).GetB();
    } else if (strName == "can_we_show_grid") {
        m_BackendDatas.canWeShowGrid = ct::ivariant(strValue).GetB();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainBackend::m_build() {
    return false;
}

bool MainBackend::m_CreateVulkanWindow() {
    m_VulkanWindowPtr = GaiApi::VulkanWindow::Create(INITIAL_WIDTH, INITIAL_HEIGHT, Lumo_Prefix " beta", false);
    if (m_VulkanWindowPtr != nullptr && m_VulkanWindowPtr->getWindowPtr() != nullptr) {
        glfwSetWindowCloseCallback(m_VulkanWindowPtr->getWindowPtr(), glfw_window_close_callback);
    }
    return (m_VulkanWindowPtr != nullptr);
}

void MainBackend::m_DestroyVulkanWindow() {
    m_VulkanWindowPtr->Unit();
    m_VulkanWindowPtr.reset();
}

void MainBackend::m_InitFilesTracker() {
    FilesTrackerSystem::Instance()->init();                                                  //
    m_InitFilesTracker(std::bind(&MainBackend::m_UpdateFiles, this, std::placeholders::_1),  //
        std::list<std::string>{".", "shaders", "debug", "debug/shaders"});
}

void MainBackend::m_UnitFilesTracker() {
    FilesTrackerSystem::Instance()->unit();
}

bool MainBackend::m_CreateVulkanCore() {
    GaiApi::VulkanCore::sVulkanShader = VulkanShader::Create();
    if (GaiApi::VulkanCore::sVulkanShader != nullptr) {
        m_VulkanCorePtr = GaiApi::VulkanCore::Create(MainBackend::Instance()->getWindow(), Lumo_Prefix, 1, Lumo_Prefix " Engine", 1, true, USE_RTX);
        return (m_VulkanCorePtr != nullptr);
    }
    return false;
}

void MainBackend::m_InitScenes() {
    SceneManager::Instance()->Init(m_VulkanCorePtr);
}

void MainBackend::m_UnitScenes() {
    SceneManager::Instance()->Unit();
}

void MainBackend::m_InitNodes() {
    NodeManager::Instance()->Init(m_VulkanCorePtr);
    UserNodeLibrary::Instance()->AnalyseRootDirectory();
}

void MainBackend::m_UnitNodes() {
    NodeManager::Instance()->Unit();
}

void MainBackend::m_InitPlugins() {
    PluginManager::Instance()->LoadPlugins(m_VulkanCorePtr);
    auto pluginPanes = PluginManager::Instance()->GetPluginsPanes();
    for (auto& pluginPane : pluginPanes) {
        if (!pluginPane.paneWeak.expired()) {
            LayoutManager::Instance()->AddPane(pluginPane.paneWeak, pluginPane.paneName, pluginPane.paneCategory, pluginPane.paneDisposal,
                pluginPane.paneRatio, pluginPane.isPaneOpenedDefault, pluginPane.isPaneFocusedDefault);
            auto plugin_ptr = std::dynamic_pointer_cast<PluginPane>(pluginPane.paneWeak.lock());
            if (plugin_ptr != nullptr) {
                plugin_ptr->SetProjectInstancePtr(ProjectFile::Instance());
            }
        }
    }
}

void MainBackend::m_UnitPlugins() {
    PluginManager::Instance()->Clear();
}

void MainBackend::m_InitSystems() {
    CommonSystem::Instance()->CreateBufferObject(m_VulkanCorePtr);
    RenderDocController::Instance()->Init();
}

void MainBackend::m_UnitSystems() {
    CommonSystem::Instance()->DestroyBufferObject();
    RenderDocController::Instance()->Unit();
}

void MainBackend::m_InitPanes() {
    if (LayoutManager::Instance()->InitPanes()) {
        // a faire apres InitPanes() sinon ConsolePane::Instance()->paneFlag vaudra 0 et changeras apres InitPanes()
        Messaging::Instance()->sMessagePaneId = ConsolePane::Instance()->GetFlag();
    }
}

void MainBackend::m_DestroyVulkanCore() {
    m_VulkanCorePtr->Unit();
    m_VulkanCorePtr.reset();
    GaiApi::VulkanCore::sVulkanShader->Unit();
    GaiApi::VulkanCore::sVulkanShader.reset();
}

bool MainBackend::m_CreateImGuiOverlay() {
    m_ImGuiOverlayPtr = ImGuiOverlay::Create(m_VulkanCorePtr, MainBackend::Instance()->getWindow());  // needed for alloc ImGui Textures
    View3DPane::Instance()->SetVulkanImGuiRenderer(m_ImGuiOverlayPtr->GetImGuiRenderer());
    View2DPane::Instance()->SetVulkanImGuiRenderer(m_ImGuiOverlayPtr->GetImGuiRenderer());
    SceneViewPane::Instance()->SetVulkanImGuiRenderer(m_ImGuiOverlayPtr->GetImGuiRenderer());
    if (m_ImGuiOverlayPtr != nullptr) {
#ifdef USE_THUMBNAILS
        ImGuiFileDialog::Instance()->SetCreateThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info) {
            if (vThumbnail_Info && vThumbnail_Info->isReadyToUpload && vThumbnail_Info->textureFileDatas) {
                m_VulkanCorePtr->getDevice().waitIdle();
                std::shared_ptr<FileDialogAsset> resPtr =
                    std::shared_ptr<FileDialogAsset>(new FileDialogAsset, [](FileDialogAsset* obj) { delete obj; });
                if (resPtr) {
                    resPtr->texturePtr = Texture2D::CreateFromMemory(m_VulkanCorePtr, vThumbnail_Info->textureFileDatas,
                        vThumbnail_Info->textureWidth, vThumbnail_Info->textureHeight, vThumbnail_Info->textureChannels);
                    if (resPtr->texturePtr) {
                        auto imguiRendererPtr = m_ImGuiOverlayPtr->GetImGuiRenderer().lock();
                        if (imguiRendererPtr) {
                            resPtr->descriptorSet = imguiRendererPtr->CreateImGuiTexture((VkSampler)resPtr->texturePtr->m_DescriptorImageInfo.sampler,
                                (VkImageView)resPtr->texturePtr->m_DescriptorImageInfo.imageView,
                                (VkImageLayout)resPtr->texturePtr->m_DescriptorImageInfo.imageLayout);
                            vThumbnail_Info->userDatas = (void*)resPtr.get();
                            m_FileDialogAssets.push_back(resPtr);
                            vThumbnail_Info->textureID = (ImTextureID)&resPtr->descriptorSet;
                        }
                        delete[] vThumbnail_Info->textureFileDatas;
                        vThumbnail_Info->textureFileDatas = nullptr;
                        vThumbnail_Info->isReadyToUpload = false;
                        vThumbnail_Info->isReadyToDisplay = true;
                        m_VulkanCorePtr->getDevice().waitIdle();
                    }
                }
            }
        });
        ImGuiFileDialog::Instance()->SetDestroyThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info) {
            if (vThumbnail_Info) {
                if (vThumbnail_Info->userDatas) {
                    m_VulkanCorePtr->getDevice().waitIdle();
                    auto assetPtr = (FileDialogAsset*)vThumbnail_Info->userDatas;
                    if (assetPtr) {
                        assetPtr->texturePtr.reset();
                        assetPtr->descriptorSet = vk::DescriptorSet{};
                        auto imguiRendererPtr = m_ImGuiOverlayPtr->GetImGuiRenderer().lock();
                        if (imguiRendererPtr) {
                            imguiRendererPtr->DestroyImGuiTexture(&assetPtr->descriptorSet);
                        }
                    }
                    m_VulkanCorePtr->getDevice().waitIdle();
                }
            }
        });
#endif  // USE_THUMBNAILS
        return true;
    }
    return false;
}

void MainBackend::m_DestroyImGuiOverlay() {
    m_ImGuiOverlayPtr->Unit();
    m_ImGuiOverlayPtr.reset();
}

bool MainBackend::m_CreateRenderers() {
    m_DisplaySizeQuadRendererPtr = DisplaySizeQuadRenderer::Create(m_VulkanCorePtr);
    if (m_DisplaySizeQuadRendererPtr != nullptr) {
        return true;
    }
    return false;
}

void MainBackend::m_DestroyRenderers() {
    m_DisplaySizeQuadRendererPtr.reset();
}

void MainBackend::m_DeleteNodesIfAnys() {
    if (NodeManager::Instance()->m_RootNodePtr) {
        NodeManager::Instance()->m_RootNodePtr->DestroyNodesIfAnys();
    }
}
