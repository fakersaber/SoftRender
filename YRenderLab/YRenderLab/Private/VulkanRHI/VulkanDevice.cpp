#include <Public/VulkanRHI/VulkanDevice.h>
#include <Public/VulkanRHI/VulkanRHI.h>
#include <Public/VulkanRHI/VulkanQueue.h>


VulkanDevice::VulkanDevice(VulkanRHI* InRHI, VkPhysicalDevice InGpu)
	: VkRHI(InRHI)
	, LogicalDevice(VK_NULL_HANDLE)
	, PhysicalDevice(InGpu)
	, GfxQueue(nullptr)
	, ComputeQueue(nullptr)
	, TransferQueue(nullptr)
{
	std::memset(&PhysicalFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
}

VulkanDevice::~VulkanDevice() {

	if (LogicalDevice != VK_NULL_HANDLE){
		vkDestroyDevice(LogicalDevice, nullptr);
		LogicalDevice = VK_NULL_HANDLE;

		delete TransferQueue;
		delete ComputeQueue;
		delete GfxQueue;
	}
}

bool VulkanDevice::QueryGPU(){

	VulkanDevice::GetDeviceExtensionsAndLayers(PhysicalDevice, DeviceExtensions, DeviceLayers);
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProperties);

	//�����Կ�
	bool bIsDiscrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	bool bIsNvida = deviceProperties.vendorID == static_cast<uint32_t>(VenderID::Nvidia);

	uint32_t QueueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, nullptr);
	assert(QueueCount >= 1);
	QueueFamilyProps.resize(QueueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, QueueFamilyProps.data());
	return bIsDiscrete && bIsNvida;
}

void VulkanDevice::InitGPU() {

	// Query features
	vkGetPhysicalDeviceFeatures(PhysicalDevice, &PhysicalFeatures);
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);
	CreateDevice();
}


void VulkanDevice::CreateDevice() {
	assert(LogicalDevice == VK_NULL_HANDLE);
	std::vector<VkDeviceQueueCreateInfo> QueueFamilyInfos;
	int32_t GfxQueueFamilyIndex = -1;
	int32_t ComputeQueueFamilyIndex = -1;
	int32_t TransferQueueFamilyIndex = -1;
	printf("Found %lld Queue Families\n", QueueFamilyProps.size());

	uint32_t NumPriorities = 0;

	for (auto FamilyIndex = 0; FamilyIndex < QueueFamilyProps.size(); ++FamilyIndex) {
		const VkQueueFamilyProperties& CurrProps = QueueFamilyProps[FamilyIndex];
		bool bIsValidQueue = false;

		//Graph pipeline
		if ((CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT && GfxQueueFamilyIndex == -1) {
			GfxQueueFamilyIndex = FamilyIndex;
			bIsValidQueue = true;
		}

		//compoute pipeline
		if ((CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT && ComputeQueueFamilyIndex == -1) {
			//��ǰ����δ����ʼ��ʱ�Ÿ�ֵ
			if (!bIsValidQueue) {
				ComputeQueueFamilyIndex = FamilyIndex;
				bIsValidQueue = true;
			}
		}

		//transfer �ض�queue
		if ((CurrProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
			//��ǰ����δ����ʼ��ʱ�Ÿ�ֵ
			if (!bIsValidQueue) {
				TransferQueueFamilyIndex = FamilyIndex;
				bIsValidQueue = true;
			}
		}

		if (!bIsValidQueue) {
			printf("Skipping unnecessary Queue Family %d: %d queues\n", FamilyIndex, CurrProps.queueCount);
			continue;
		}

		VkDeviceQueueCreateInfo CurrQueue = {};
		CurrQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		CurrQueue.queueFamilyIndex = FamilyIndex;
		CurrQueue.queueCount = CurrProps.queueCount; //һ�����͵Ķ����������̶�
		NumPriorities += CurrQueue.queueCount; //ÿһ�����ж�Ҫ��һ����Ӧ��float����ָ�����ȼ�,����priorities������ΪQueue������

		QueueFamilyInfos.emplace_back(CurrQueue);
	}

	std::vector<float> QueuePriorities;
	QueuePriorities.resize(NumPriorities);
	float* CurrentPriority = QueuePriorities.data();

	for (auto& CurrQueue : QueueFamilyInfos) {
		CurrQueue.pQueuePriorities = CurrentPriority;
		const VkQueueFamilyProperties& CurrProps = QueueFamilyProps[CurrQueue.queueFamilyIndex];

		//�ɶ�ÿ��Priorities������ֵ����������ֵ
		for (int32_t QueueIndex = 0; QueueIndex < static_cast<int32_t>(CurrProps.queueCount); ++QueueIndex) {
			*CurrentPriority++ = 1.f;
		}
	}


	VkDeviceCreateInfo DeviceInfo = {};
	DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	DeviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	DeviceInfo.enabledLayerCount = static_cast<uint32_t>(DeviceLayers.size());
	DeviceInfo.ppEnabledLayerNames = (DeviceInfo.enabledLayerCount > 0) ? DeviceLayers.data() : nullptr;

	DeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueFamilyInfos.size());
	DeviceInfo.pQueueCreateInfos = QueueFamilyInfos.data();

	DeviceInfo.pEnabledFeatures = &PhysicalFeatures;


	// Create the device

	//the third param is allocation pointer to store the logical device handle in
	VkResult Result = vkCreateDevice(PhysicalDevice, &DeviceInfo, nullptr, &LogicalDevice);

	if (Result != VK_SUCCESS) {
		std::cerr << "Vulkan device creation failed" << std::endl;
	}

	//Gfx Queue
	if (GfxQueueFamilyIndex == -1) {
		std::cerr << "Queue has Error" << std::endl;
		assert(GfxQueueFamilyIndex == -1);
	}
	GfxQueue = new VulkanQueue(this, GfxQueueFamilyIndex);

	//Compute Queue
	if (ComputeQueueFamilyIndex == -1) {
		std::cerr << "Queue has Error" << std::endl;
		assert(ComputeQueueFamilyIndex == -1);
	}
	ComputeQueue = new VulkanQueue(this, ComputeQueueFamilyIndex);

	//Transfer Queue
	if (TransferQueueFamilyIndex == -1) {
		std::cerr << "Queue has Error" << std::endl;
		assert(TransferQueueFamilyIndex == -1);
	}
	TransferQueue = new VulkanQueue(this, TransferQueueFamilyIndex);
}



VulkanQueue* VulkanDevice::SetupPresentQueue(VkSurfaceKHR Surface){
	VkBool32 bSupportsPresent = VK_FALSE;
	const auto FamilyIndex = GfxQueue->GetFamilyIndex();
	vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, FamilyIndex, Surface, &bSupportsPresent);
	assert(bSupportsPresent);
	//just gfxQueue
	return  GfxQueue;
}
