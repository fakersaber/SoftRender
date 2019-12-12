#ifndef _YRENDER_VIEWER_FORWARDRASTER_H_
#define _YRENDER_VIEWER_FORWARDRASTER_H_

#include <Public/Viewer/Raster.h>
#include <Public/YGM/Transform.h>
#include <Public/OpenGLRHI/GLVAO.h>
#include <Public/OpenGLRHI/GLShader.h>
#include <Public/OpenGLRHI/GLTexture.h>

#include <vector>
#include <map>

namespace YRender {
	class YObject;
	class TriMesh;
	class BSDF_blinnPhong;
	class BSDF_DiffuseSpecular;
	class GlfwWindow;

	class ForwardRaster : public Raster {
	public:
		ForwardRaster(std::shared_ptr<Scene> scene, std::shared_ptr<GlfwWindow> pGlwindow);

	public:
		virtual void Draw() override;
		virtual void Initial() override;
		void SetCurShader(const GLShader& shader) { curShader = shader; };
	protected:
		virtual ~ForwardRaster() = default;
		virtual void Visit(std::shared_ptr<YObject> obj);
		virtual void Visit(std::shared_ptr<BSDF_blinnPhong> bsdf);
		void Visit(std::shared_ptr<TriMesh> mesh);
		void InitShaderPbrBlinnPhong();
		void InitShader_Skybox();
		void DrawEnvironment();
		

	private:
		std::vector<YGM::Transform> modelVec;
		//暂时只有Component持有Mesh，注意容器没有清除
		//std::map<std::weak_ptr<TriMesh>, VAO, std::owner_less<std::weak_ptr<TriMesh>>> mesh2VAO;
		//std::map<std::weak_ptr<Image>, GLTexture, std::owner_less<std::weak_ptr<Image>>> img2tex;


		GLShader BlinnPhongShader;
		GLShader curShader;
		GLShader shader_skybox;



	//test
	public:
		GLTexture IBLmap;
	};
}

#endif