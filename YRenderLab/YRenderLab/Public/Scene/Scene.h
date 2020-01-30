#ifndef _YRENDER_SCENE_SCENE_H_
#define _YRENDER_SCENE_SCENE_H_

#include <Public/Basic/YHeapObject.h>
#include <vector>

class Camera;
class YObject;
class Image;
class LightComponent;

class Scene : public YHeapObject {
public:
	Scene(std::shared_ptr<YObject> root, std::shared_ptr<Camera> camera, std::shared_ptr<Image> EnviromentImg) :
		root(root),
		camera(camera),
		EnviromentImg(EnviromentImg)
	{
	}

protected:
	virtual ~Scene() = default;

public:
	std::shared_ptr<Image> GetEnviromentImg() const { return EnviromentImg; }
	std::shared_ptr<YObject> GetRoot() const { return root; }
	std::shared_ptr<Camera> GetCamera() const { return camera; }
	std::vector<std::shared_ptr<LightComponent>> GetLightComponents() const;


private:
	std::shared_ptr<YObject> root;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Image> EnviromentImg;
};


#endif