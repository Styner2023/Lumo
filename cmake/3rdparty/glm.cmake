add_definitions(-DGLM_FORCE_SILENT_WARNINGS)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glm)

set_target_properties(uninstall PROPERTIES FOLDER CMakePredefinedTargets)

set(GLM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glm/glm)

set_target_properties(glm_shared PROPERTIES FOLDER 3rdparty/Shared)
set_target_properties(glm_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(glm_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(glm_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
set_target_properties(glm_shared PROPERTIES RUNTIME_OUTPUT_NAME_DEBUG "glmd")
set(GLM_LIBRARIES ${GLM_LIBRARIES} glm_shared)
