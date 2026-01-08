#pragma once

#include <glad/glad.h>
#include <vector>

class VBO {
public:
	unsigned int ID;
	VBO(const std::vector<float>& vertices);
	void bind();
	void unbind();
	VBO(const VBO&) = delete;
	VBO& operator=(const VBO&) = delete;
	~VBO();

};