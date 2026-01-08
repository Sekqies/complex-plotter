#pragma once
#include <string>
#include <iostream>
#include <stb_image/stb_image.h>
#include <shader/shader.h>
#include <glad/glad.h>

using std::string;

class Texture {
public:
	unsigned int ID;
	unsigned int texture_code;
	GLenum format;
	GLenum target;
	/// <summary>
	/// Initializes an OpenGL texture with an image
	/// </summary>
	/// <param name="image_path">The path to the image whose data will be used to make the texture</param>
	/// <param name="format">The format of byte encoding in the image. Defaults to GL_RGB</param>
	/// <param name="target">The internal OpenGL target (defaults to GL_TEXTURE_2D)</param>
	Texture(const string& image_path, const GLenum target = GL_TEXTURE_2D, const GLenum min_filter = GL_LINEAR_MIPMAP_NEAREST, const GLenum mag_filter = GL_LINEAR, const GLenum texture_wrap = GL_REPEAT);
	void attach_to_shader(Shader& shader_program, const string& name, const int internal_code);
	void activate();
	void bind();
	~Texture();
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// move semantics nonsense
	Texture(Texture&& other) noexcept;

	Texture& operator=(Texture&& other) noexcept;
private:
	int width;
	int height;
	int nr_channels;
};