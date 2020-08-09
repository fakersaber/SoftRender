﻿#ifndef _YRENDER_YCORE_H_
#define _YRENDER_YCORE_H_

#define NOMINMAX

#define YRENDER_REVERSE

//Window Define
#define USE_GLFWWINDOW 1

//RHI Define
#define SOFT_RENDER 0
#define OPENGL_RENDER 0
#define VULKAN_RENDER 1

//Platform Define
#define USE_WINDOWS_PLATFORM 1

#include <memory>
#include <Public/YGM/Transform.h>


namespace CoreDefine {
	enum class StaticVAOType : uint8_t {
		Sphere,
		Plane,
		Screen,
		Cube
	};

	enum class MaterialID : uint8_t {
		BSDF_StandardPBR = 0,
		BSDF_Emission = 1
	};


	static const float data_ScreenVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	static const float data_Flip_ScreenVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,

		-1.0f,  1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 0.0f
	};

	//显然是正交矩阵，逆矩阵与原矩阵相同
	static const YGM::Transform ModelReverse = YGM::Transform(
		Vector4(1.f, 0.f, 0.f, 0.f),
		Vector4(0.f, 0.f, 1.f, 0.f),
		Vector4(0.f, 1.f, 0.f, 0.f),
		Vector4(0.f, 0.f, 0.f, 1.f)
	);

	static const int maxPointLights = 0;

	static const int maxDirectionalLights = 1;

	static const int maxSpotLights = 0;

}


class GlfwWindow;
class RenderWindow;
class IWindowInterface;
class RHI;
class RHIViewport;

class YCore {
public:
	YCore(const YCore& rhs) = delete;
	YCore& operator=(const YCore& rhs) = delete;
	static YCore* GetCore();
	void Init(const int width, const int height);
	void Shutdown();
	void Tick();

	std::shared_ptr<GlfwWindow> GetGLWindow() const {
		return pGLInstance;
	}

private:
	YCore();
	~YCore();

private:
	//---------------------
	//#TODO remove
	std::shared_ptr<GlfwWindow> pGLInstance;
	RenderWindow* _RenderWindow;
	//-----------------------

	std::shared_ptr<IWindowInterface> SurfaceRenderWindow;
	std::shared_ptr<RHI> RenderRHI;
};



#endif