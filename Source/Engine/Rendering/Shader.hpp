#pragma once
#include <string>

class Shader {
public:
	void Compile(const std::string& vertexPath, const std::string& fragmentPath);
	void Use();
	void SetMatrix4(const std::string& name, const float* value);
};