#include <Public/GlfwApplication.h>
#include <Public/YRenderLabConfig.h>

#if OPENGL_RENDER
#include <Public/OpenGLRHI/GlfwWindow.h>
#endif

#if VULKAN_RENDER
#include <Public/VulkanRHI/VulkanRHI.h>
#define GLFW_INCLUDE_VULKAN
#endif

#include <Public/3rdPart/GLFW/glfw3.h>

#if USE_WINDOWS_PLATFORM
#define GLFW_EXPOSE_NATIVE_WIN32
#include <Public/3rdPart/GLFW/glfw3native.h>
#endif


void GlfwApplication::Init(const int width, const int height) {
	//--------------Init RHI--------------//
#if OPENGL_RENDER
	this->pGLInstance = New<GlfwWindow>();
#elif VULKAN_RENDER
	RenderRHI = new VulkanRHI();
	RenderRHI->Init();
#endif 


	//--------------Init Window--------------//
	glfwInit();
#if OPENGL_RENDER
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif VULKAN_RENDER
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif 

	window = glfwCreateWindow(width, height, "YRender", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	//it��s the Targa bitmap format (.tga). It stores the pixel data in BGRA format.so Use BGRA?
	RenderPipeline = RenderRHI->RHICreateRenderPipeline(glfwGetWin32Window(window), width, height, PF_B8G8R8A8);
}

void GlfwApplication::Shutdown() {
	delete RenderPipeline;
	delete RenderRHI;
	glfwTerminate();
}

void GlfwApplication::Update(){
	//����ע��Ļص�����
	glfwPollEvents();
}

void GlfwApplication::Render(){

	RenderPipeline->Render();

}

void GlfwApplication::Tick() {
	while (!glfwWindowShouldClose(window)) {
		Update();
		Render();
	}
}

