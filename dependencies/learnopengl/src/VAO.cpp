#include <VAO/vao.h>
#include <glad/glad.h>

VAO::VAO() {
	glGenVertexArrays(1, &ID);
}

/// <summary>
/// Sets pointer attributes originating from a VBO
/// </summary>
/// <param name="vbo">The VBO with attached vertices</param>
/// <param name="location">The location of the attribute in its corresponding shader</param>
/// <param name="size_component">The number of elements in each component</param>
/// <param name="type">The type of the variables being passed</param>
/// <param name="stride">The distance in the vertices array between each attribute</param>
/// <param name="offset">The offset from the beginning of the array of the correspoding attribute</param>
void VAO::set_attributes(VBO& vbo, const unsigned int location, const unsigned int size_component, const GLenum type, const unsigned int stride, const void* offset) {
	vbo.bind();
	const GLboolean normalize = type != GL_FLOAT;
	glVertexAttribPointer(location, size_component, type, normalize, stride, offset);
	glEnableVertexAttribArray(location);
	vbo.unbind();
}

void VAO::bind() {
	glBindVertexArray(ID);
}

void VAO::unbind() {
	glBindVertexArray(0);
}

VAO::~VAO() {
	glDeleteVertexArrays(1, &ID);
}
