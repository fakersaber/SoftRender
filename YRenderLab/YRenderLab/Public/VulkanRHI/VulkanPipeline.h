#ifndef _YRENDER_VKRHI_VULKANPIPELINE_H_
#define _YRENDER_VKRHI_VULKANPIPELINE_H_


#include <Public/VulkanRHI/VulkanRHI.h>

class IPipeline;
class VulkanSwapChain;
class VulkanRHI;

//Resource
struct VulkanTextureResource;
struct VulkanIndexBufferResource;




class VulkanPipeline : public IPipeline {
	struct VulkanFrameSemaphores {
		VkSemaphore presentComplete;// Swap chain image presentation
		VkSemaphore renderComplete;// Command buffer submission and execution
	};


public:
	virtual ~VulkanPipeline();
	VulkanPipeline(void* InWindowHandle, VulkanRHI* InRHI, uint32_t InSizeX, uint32_t InSizeY, EPixelFormat InPixelFormat, bool bIsSRGB);

	virtual void BeginFrame() override;
	virtual void Render() override;
	virtual void EndFrame() override;

private:
	void* WindowHandle;
	//[Resource ref]
	VulkanRHI* RHI;
	uint32_t SizeX;
	uint32_t SizeY;
	EPixelFormat PixelFormat;

	//[Resource management]
	VulkanSwapChain* SwapChain;
	// Command buffer pool
	VkCommandPool CommandlBufferPool;
	std::vector<VkCommandBuffer> CommandBuffers;
	// Synchronization semaphores
	VulkanFrameSemaphores semaphores;
	// Contains command buffers and semaphores to be presented to the queue
	VkSubmitInfo submitInfo;
	std::vector<VkFence> WaitFences;

	//[Resource Build]
	VulkanTextureResource SwapChainDepthStencilResource;
	// Global render pass for frame buffer writes
	VkRenderPass BackBufferRenderPass;
	// Pipeline cache object
	VkPipelineCache pipelineCache;
	// List of available frame buffers (same as number of swap chain images)
	std::vector<VkFramebuffer> SwapChainFrameBuffers;

	//IndexBuffer
	VulkanIndexBufferResource TriangleIndexBuffer;
};


#endif