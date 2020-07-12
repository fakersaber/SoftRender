#ifndef _YRENDER_SCENE_YOBJECT_H_
#define _YRENDER_SCENE_YOBJECT_H_

#include <string>
#include <unordered_map>
#include <vector>

#include <Public/Basic/YHeapObject.h>
#include <Public/Basic/Node.h>
#include <Public/YGM/Vector3.hpp>

class Component;
class YObject : public Node<YObject> {
public:
	//��keyֵΪ��������ʱ��ʹ��reference_wrapper��װ�������ֵ����
	using TypeInfoRef = std::reference_wrapper<const std::type_info>;

	struct Hasher {
		std::size_t operator()(TypeInfoRef type) const {
			return type.get().hash_code();
		}
	};

	struct EqualTo {
		bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const {
			return lhs.get() == rhs.get();
		}
	};
public:
	YObject(const std::string& objname = "default", std::shared_ptr<YObject> parent = nullptr) :
		name(objname),
		Node(parent)
	{

	}

protected:
	YObject() = delete;
	virtual ~YObject() = default;

public:

	void AttachComponent(const std::shared_ptr<Component> component);

	void DetachComponent(const std::shared_ptr<Component> component);

	Vector3 GetObjectWorldForward();

	const std::string& GetYObjectName() { return name; }

	void PrintNode(const std::string& PreSpace = std::string("")) {
		std::cout << PreSpace + name << std::endl;
		for (auto child : GetChildrens()) {
			child->PrintNode(PreSpace + std::string("----"));
		}
	}

	const std::unordered_map<TypeInfoRef, std::shared_ptr<Component>, Hasher, EqualTo>& GetComponents() const { return components; };

	template <typename ComponentType>
	const std::shared_ptr<ComponentType> GetComponent() const;

	template<typename ComponentType>
	const std::vector<std::shared_ptr<ComponentType>> GetComponentsInChildren();
	//template <typename ComponentType>
	//const std::shared_ptr<ComponentType> GetComponentInChildren()

private:
	friend YHeapObject;

	std::unordered_map<TypeInfoRef, std::shared_ptr<Component>, Hasher, EqualTo> components;

	std::string name;
};


template<typename ComponentType>
const std::shared_ptr<ComponentType> YObject::GetComponent() const {
	static_assert(std::is_base_of_v<Component, ComponentType>, "Component Type Error!");
	auto iter = components.find(typeid(ComponentType));
	if (iter == components.end())
		return nullptr;

	//#TODO������ֱ��ʹ��ǿ������ת��������Cast��������ת���ܹ�
	return Cast<ComponentType>(iter->second);
}

template<typename ComponentType>
const std::vector<std::shared_ptr<ComponentType>> YObject::GetComponentsInChildren() {
	std::vector<std::shared_ptr<ComponentType>> RetComponents;
	

	ForEachNode([&RetComponents](std::shared_ptr<YObject>&& obj) {
		if (auto ComponentOfType = obj->GetComponent<ComponentType>()) {
			RetComponents.emplace_back(ComponentOfType);
		}
	});

	return RetComponents;
}



#endif