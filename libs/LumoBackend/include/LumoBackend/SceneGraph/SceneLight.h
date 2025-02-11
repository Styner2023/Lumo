/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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
#pragma warning(disable : 4251)

#include <vector>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <LumoBackend/SceneGraph/SceneMesh.hpp>
#include <LumoBackend/Interfaces/GizmoInterface.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

#pragma warning(push)
#pragma warning(disable:4201)   // suppress even more warnings about nameless structs
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

class SceneLight;
typedef std::shared_ptr<SceneLight> SceneLightPtr;
typedef std::weak_ptr<SceneLight> SceneLightWeak;

enum class LightTypeEnum : uint8_t
{
	NONE = 0,
	POINT,
	DIRECTIONNAL,
	SPOT,
	AREA,
	Count
};
inline static std::string GetStringFromLightTypeEnum(const LightTypeEnum& vLightTypeEnum)
{
	static std::array<std::string, (uint8_t)LightTypeEnum::Count> LightTypeString = {
		"NONE",
		"POINT",
		"DIRECTIONNAL",
		"SPOT",
		"AREA"
	};
	if (vLightTypeEnum != LightTypeEnum::Count)
		return LightTypeString[(int)vLightTypeEnum];
	LogVarDebugError("Error, one LightTypeEnum have no corresponding string, return \"None\"");
	return "NONE";
}

inline static LightTypeEnum GetLightTypeEnumFromString(const std::string& vNodeTypeString)
{
	if (vNodeTypeString == "NONE") return LightTypeEnum::NONE;
	else if (vNodeTypeString == "POINT") return LightTypeEnum::POINT;
	else if (vNodeTypeString == "DIRECTIONNAL") return LightTypeEnum::DIRECTIONNAL;
	else if (vNodeTypeString == "SPOT") return LightTypeEnum::SPOT;
	else if (vNodeTypeString == "AREA") return LightTypeEnum::AREA;
	return LightTypeEnum::NONE;
}

class LUMO_BACKEND_API SceneLight : public GizmoInterface
{
public:
	static SceneLightPtr Create();
	static std::string GetStructureHeader();

private:
	SceneLightWeak m_This; 

public:
    SceneLight() = default;
    virtual ~SceneLight() = default;

	// std430
	struct LightDatasStruct {
		glm::mat4x4 lightGizmo = glm::mat4x4(1.0f);
		glm::mat4x4 lightView = glm::mat4x4(1.0f);
		ct::fvec4 lightColor = 1.0f;
		float lightIntensity = 1.0f;
		int lightType = (int)LightTypeEnum::DIRECTIONNAL;
		float orthoSideSize = 30.0f;
		float orthoRearSize = 1000.0f;
		float orthoDeepSize = 1000.0f;
		float perspectiveAngle = 45.0f;
		float lightActive = 1.0f;
		float is_inside = 0.0f;
	} lightDatas;
	
public:
	float* GetGizmoFloatPtr() override;

	bool NeedUpdateCamera();

	void AdaptIconColor();
};