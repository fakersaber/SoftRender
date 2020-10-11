#pragma once

#include <string>

namespace YFG {
	class ResourceNode {
	public:
		//������ֵ������ֵ�����ַ����ܻ����һ�ι��캯������ڴ������ڴ濽��������ͳһ��ֵ����
		ResourceNode(std::string InName)
			: name{ std::move(InName) } 
		{ 

		}

		const std::string& Name() const noexcept { return name; }
	private:
		std::string name;
	};
}