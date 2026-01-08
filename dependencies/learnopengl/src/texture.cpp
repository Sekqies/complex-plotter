#include <texture/texture.h>

/// <summary>
/// Initializes an OpenGL texture with an image
/// </summary>
/// <param name="image_path">The path to the image whose data will be used to make the texture</param>
/// <param name="format">The format of byte encoding in the image. Defaults to GL_RGB</param>
/// <param name="target">The internal OpenGL target (defaults to GL_TEXTURE_2D)</param>
inline Texture::Texture(const string& image_path, const GLenum target, const GLenum min_filter, const GLenum mag_filter, const GLenum texture_wrap) {
	this->target = target;
	glGenTextures(1, &ID);
	glBindTexture(target, ID);
	format = GL_RGB;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(image_path.c_str(), &width, &height, &nr_channels, 0);
	if (!data) {
		std::cerr << "Error when loading Texture: failed at loading image " << image_path;
		return;
	}
	if (nr_channels == 1)
		format = GL_RED;
	else if (nr_channels == 2)
		format = GL_RG;
	else if (nr_channels == 3)
		format = GL_RGB;
	else if (nr_channels == 4)
		format = GL_RGBA;
	glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(target);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(false);
}

inline void Texture::attach_to_shader(Shader& shader_program, const string& name, const int internal_code) {
	shader_program.use();
	shader_program.setInt(name, internal_code);
	texture_code = internal_code;
}

inline void Texture::activate() {
	glActiveTexture(GL_TEXTURE0 + texture_code);
	bind();
}

inline void Texture::bind() {
	glBindTexture(target, ID);
}

inline Texture::~Texture() {
	glDeleteTextures(1, &ID);
}

// move semantics nonsense
inline Texture::Texture(Texture&& other) noexcept
	: ID(other.ID), texture_code(other.texture_code),
	format(other.format), target(other.target),
	width(other.width), height(other.height), nr_channels(other.nr_channels)
{
	other.ID = 0;
}

inline Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (ID != 0) glDeleteTextures(1, &ID);

		ID = other.ID;
		texture_code = other.texture_code;
		format = other.format;
		target = other.target;
		width = other.width;
		height = other.height;
		nr_channels = other.nr_channels;

		other.ID = 0;
	}
	return *this;
}
