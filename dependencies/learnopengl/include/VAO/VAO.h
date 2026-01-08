#include <glad/glad.h>
#include <VBO/VBO.h>
#include <vector>


class VAO {
public:
	unsigned int ID;
	VAO();
	/// <summary>
	/// Sets pointer attributes originating from a VBO
	/// </summary>
	/// <param name="vbo">The VBO with attached vertices</param>
	/// <param name="location">The location of the attribute in its corresponding shader</param>
	/// <param name="size_component">The number of elements in each component</param>
	/// <param name="type">The type of the variables being passed</param>
	/// <param name="stride">The distance, in bytes, between each attribute</param>
	/// <param name="offset">The offset from the beginning of the array of the correspoding attribute</param>
	VAO(const VAO&) = delete;
	VAO& operator=(const VAO&) = delete;
	VAO(VAO&& other) noexcept {
		this->ID = other.ID;
		other.ID = 0; 
	}
	void set_attributes(VBO& vbo, const unsigned int location, const unsigned int size_component, const GLenum type, const unsigned int stride, const void* offset);
	void bind();
	void unbind();

	~VAO();
};