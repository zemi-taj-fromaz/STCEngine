#include "AppImpl.h"

#include "Platform/Vulkan/AppVulkanImpl.h"

AppImpl* AppImpl::create() {

	//switch (IMPL_MACRO)
	//{
	//case1:return nesto
	//case2:return nestodrugo
	//}

	return new  AppVulkanImpl();
}

void AppImpl::push_layer(std::shared_ptr<Layer>& layer)
{
	m_LayerStack.push_layer(layer);
}
