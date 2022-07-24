// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PluginInterface.h"

#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanWindow.h>
#include <Graph/Base/NodeSlot.h>

PluginInterface::~PluginInterface()
{
	Unit();
}

bool PluginInterface::Init(
	vkApi::VulkanCoreWeak vVulkanCoreWeak,
	FileHelper* vFileHelper, 
	CommonSystem* vCommonSystem,
	ImGuiContext* vContext,
	SlotColor *vSlotColor,
	ImGui::CustomStyle* vCustomStyle)
{
	// on transfere les singleton dans l'espace memoire static de la dll
	// ca evitera dans recreer des vides et d'avoir des erreurs partout
	// car les statics contenus dans ces classes sont null quand ils arrivent ici
	
	m_VulkanCoreWeak = vVulkanCoreWeak;
	auto corePtr = m_VulkanCoreWeak.getValidShared();
	if (corePtr)
	{
		if (vkApi::VulkanCore::sAllocator == nullptr)
		{
			assert(vFileHelper);
			assert(vCommonSystem);
			assert(vContext);
			assert(vSlotColor);
			assert(vCustomStyle);

			iSinAPlugin = true;
			corePtr->setupMemoryAllocator();
			FileHelper::Instance(vFileHelper);
			CommonSystem::Instance(vCommonSystem);
			ImGui::SetCurrentContext(vContext);
			NodeSlot::GetSlotColors(vSlotColor);
			ImGui::CustomStyle::Instance(vCustomStyle);
			vkApi::VulkanCore::sVulkanShader = VulkanShader::Create();
		}
		else
		{
#ifndef USE_STATIC_LINKING_OF_PLUGINS	
			CTOOL_DEBUG_BREAK;
			LogVarInfo("le static VulkanCore::sAllocator n'est pas null..");
			// a tien ? ca a changé ?
#endif // USE_STATIC_LINKING_OF_PLUGINS
		}

		ActionAfterInit();

		return true;
	}

	return false;
}

void PluginInterface::Unit()
{
	auto corePtr = m_VulkanCoreWeak.getValidShared();
	if (corePtr)
	{
		corePtr->getDevice().waitIdle();

#ifndef USE_STATIC_LINKING_OF_PLUGINS	
		// for avoid issue when its a normal class 
		// like PlugiManager who inherit from PluginInterface
		if (iSinAPlugin)
		{
			ImGui::CustomStyle::Instance(nullptr, true);
			NodeSlot::GetSlotColors(nullptr, true);
			CommonSystem::Instance(nullptr, true);
			FileHelper::Instance(nullptr, true);

			ImGui::SetCurrentContext(nullptr);

			vkApi::VulkanCore::sVulkanShader.reset();

			corePtr = nullptr;
			vmaDestroyAllocator(vkApi::VulkanCore::sAllocator);
		}
#endif
	}
}

void  PluginInterface::ActionAfterInit()
{

}