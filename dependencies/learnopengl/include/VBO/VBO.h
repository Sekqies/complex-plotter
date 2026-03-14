#pragma once

#include <vector>

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

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