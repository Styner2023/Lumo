cmake_minimum_required(VERSION 3.1)

set(PROJECT LumoBackend)

project(${PROJECT} CXX)

enable_language(C CXX)

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "")

file(GLOB_RECURSE PROJECT_INC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX include FILES ${PROJECT_INC_RECURSE})

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

## other define
add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DUSE_STD_FILESYSTEM)
add_definitions(-DUSE_VULKAN)
add_definitions(-DVULKAN)

if(UNIX)
	if(APPLE)
		add_definitions(-DAPPLE)
	else()
		add_definitions(-DLINUX)
	endif()
	add_definitions(-DUNIX)
elseif(WIN32)
	add_definitions(-DWIN32)
	if(MINGW)
		add_definitions(-DMINGW32)
	endif()
endif()

if(MSVC)
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055")
	add_definitions(-D__STDC__)
	add_definitions(-DMSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11") ## -std=gnu++0x
endif ()

add_definitions(-DUSE_IM_TOOLS)
add_definitions(-DUSE_IM_GUIZMO)
add_definitions(-DUSE_IMCOOL_BAR)
add_definitions(-DUSE_IMGUI_MARKDOW)
add_definitions(-DUSE_IMGUI_FILE_DIALOG)
add_definitions(-DUSE_IMGUI_NODE_EDITOR)
add_definitions(-DUSE_IMGUI_COLOR_TEXT_EDIT)

set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
add_library(${PROJECT} 
	${PROJECT_SRC_RECURSE} 
	${PROJECT_INC_RECURSE})
target_compile_definitions(${PROJECT} INTERFACE BUILD_LUMO_BACKEND_SHARED_LIBS)
target_compile_definitions(${PROJECT} PRIVATE BUILD_GAIA_SHARED_LIBS)	
target_compile_definitions(${PROJECT} PRIVATE BUILD_CTOOLS_SHARED_LIBS)	
target_compile_definitions(${PROJECT} PRIVATE BUILD_IMGUI_PACK_SHARED_LIBS)
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(USE_DEBUG_SANITIZER)
	target_compile_options(${PROJECT} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>)
	target_link_options(${PROJECT} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>)
	message("Address Sanitizer enabled for projet ${PROJECT}")
endif()

target_include_directories(${PROJECT} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${GAIA_INCLUDE_DIRS}
	${EFSW_INCLUDE_DIR}
)

target_link_libraries(${PROJECT}
	${GAIA_LIBRARIES}
	${EFSW_LIBRARIES}
)

install(TARGETS ${EFSW_LIBRARIES} RUNTIME DESTINATION / COMPONENT APP_LIBS_${EFSW_LIBRARIES})

set(${PROJECT}_Output_Name "${PROJECT}$<$<CONFIG:Debug>:d>")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_Output_Name}")

set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)

set(LUMO_BACKEND_INCLUDE_DIRS 
	${CMAKE_CURRENT_SOURCE_DIR}/include 
	${GAIA_INCLUDE_DIRS} 
	${EFSW_INCLUDE_DIR}
PARENT_SCOPE)

set(LUMO_BACKEND_LIBRARIES 
	${PROJECT}
	${GAIA_LIBRARIES} 
	${EFSW_LIBRARIES}
PARENT_SCOPE)

set(LUMO_BACKEND_INSTALL_LIBRARIES
	${PROJECT}
	${EFSW_LIBRARIES}
	${GAIA_INSTALL_LIBRARIES} 
PARENT_SCOPE)

set(LUMO_BACKEND_LIB_DIR 
	${CMAKE_CURRENT_BINARY_DIR} 
PARENT_SCOPE)

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

add_custom_command(
	TARGET ${PROJECT} PRE_BUILD
	COMMAND BuildInc -prefix=${PROJECT} 100000:10 $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/include/LumoBackend/Headers/${PROJECT}Build.h>
	DEPENDS ${PROJECT}
)
