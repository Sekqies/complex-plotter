#include <VBO/VBO.h>

VBO::VBO(const std::vector<float>& vertices) {
	glGenBuffers(1, &ID);
	this->bind();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}

void VBO::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
VBO::~VBO() {
	glDeleteBuffers(1, &ID);
}
