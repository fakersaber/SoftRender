#include <Public/VulkanRHI/VulkanRHI.h>
#include <Public/VulkanRHI/VulkanDevice.h>



VulkanRHI::VulkanRHI()
	:Instance(VK_NULL_HANDLE)
	, Device(nullptr)
{

}

VulkanRHI::~VulkanRHI() {

}

void VulkanRHI::Init() {

	SetupFormat();

	CreateInstance();

	SelectAndInitDevice();
}

void VulkanRHI::Shutdown() {

	Device.reset();

	vkDestroyInstance(Instance, nullptr);

	RemoveDebugLayerCallback();
}

void VulkanRHI::SetupFormat(){

	PlatformFormats[PF_B8G8R8A8].PlatformFormat = VK_FORMAT_B8G8R8A8_UNORM;
	PlatformFormats[PF_FloatRGB].PlatformFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	PlatformFormats[PF_FloatRGBA].PlatformFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
}


void VulkanRHI::CreateInstance() {
	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "YRenderLab";
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.pEngineName = "YRenderLab";
	AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo InstInfo = {};
	InstInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstInfo.pApplicationInfo = &AppInfo;

	VulkanRHI::GetInstanceLayersAndExtensions(InstanceExtensions, InstanceLayers);

	InstInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	InstInfo.ppEnabledExtensionNames = InstInfo.enabledExtensionCount > 0 ? InstanceExtensions.data() : nullptr;
	InstInfo.enabledLayerCount = static_cast<uint32_t>(InstanceLayers.size());
	InstInfo.ppEnabledLayerNames = InstInfo.enabledLayerCount > 0 ? InstanceLayers.data() : nullptr;

	//Allocator always nullptr
	VkResult Result = vkCreateInstance(&InstInfo, nullptr, &Instance);
	if (Result != VK_SUCCESS) {
		std::cerr << "Instance Create Error" << std::endl;
	}

#if (VULKAN_HAS_DEBUGGING_ENABLED && VULKAN_SUPPORTS_DEBUG_UTILS)
	SetupDebugLayerCallback();
#endif

}

void VulkanRHI::SelectAndInitDevice(){
	uint32_t GpuCount = 0;
	VkResult Result = vkEnumeratePhysicalDevices(Instance, &GpuCount, nullptr);
	if (Result != VK_SUCCESS){
		std::cerr << "Cannot find a compatible Vulkan device or driver." << std::endl;
	}

	if (GpuCount == 0) {
		std::cerr << "No GPU(s)/Driver(s) that support Vulkan were found" << std::endl;
	}

	std::vector<VkPhysicalDevice> PhysicalDevices(GpuCount);
	vkEnumeratePhysicalDevices(Instance, &GpuCount, PhysicalDevices.data());
	if (Result != VK_SUCCESS) {
		std::cerr << "Couldn't enumerate physical devices!" << std::endl;
	}

	for (auto& CurGpu : PhysicalDevices){
		auto NewDevice = std::make_shared<VulkanDevice>(this, CurGpu);
		if (NewDevice->QueryGPU()) {
			Device = NewDevice;
			break;
		}
	}

	if (Device == nullptr) {
		std::cerr << "No devices found!" << std::endl;
	}

	Device->InitGPU();

}



void VulkanRHI::SetupDebugLayerCallback(){
#if VULKAN_SUPPORTS_DEBUG_UTILS
	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = 
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
	if (CreateDebugUtilsMessengerEXT)
	{
		VkDebugUtilsMessengerCreateInfoEXT CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		CreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

		CreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		CreateInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)(void*)DebugUtilsCallback;
		VkResult Result = (*CreateDebugUtilsMessengerEXT)(Instance, &CreateInfo, nullptr, &Messenger);

		if (Result != VK_SUCCESS) {
			std::cerr << "SetupDebugLayerCallback Error" << std::endl;
		}
	}
#endif
}

VkBool32 VulkanRHI::DebugUtilsCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT MsgSeverity,
	VkDebugUtilsMessageTypeFlagsEXT MsgType,
	const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
	void* UserData)
{
	std::cerr << "validation layer: " << CallbackData->pMessage << std::endl;
	return VK_FALSE;
}


void VulkanRHI::RemoveDebugLayerCallback(){
#if VULKAN_SUPPORTS_DEBUG_UTILS
	if (Messenger != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = 
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");

		if (DestroyDebugUtilsMessengerEXT)
		{
			(*DestroyDebugUtilsMessengerEXT)(Instance, Messenger, nullptr);
		}
	}
#endif
}


