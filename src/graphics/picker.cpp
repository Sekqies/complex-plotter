#include <graphics/picker.h>

PickerResult get_hover_value(double raw_x, double raw_y, ViewState& view_state, unsigned int stack_tex, unsigned int const_tex, Shader& picker_shader, unsigned int picker_fbo) {
	glBindFramebuffer(GL_FRAMEBUFFER, picker_fbo);
	glViewport(0, 0, 1, 1);
	picker_shader.use();

	float gl_x = static_cast<float>(raw_x);
	float gl_y = view_state.height - static_cast<float>(raw_y);

	picker_shader.setVec2("u_mouse", glm::vec2(gl_x, gl_y));
	picker_shader.setVec2("u_resolution", glm::vec2(view_state.width, view_state.height));
	picker_shader.setFloat("u_range", view_state.range);
	picker_shader.setVec2("shift", view_state.shift);
	picker_shader.setFloat("time", glfwGetTime());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, stack_tex);
	picker_shader.setInt("operator_stack", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, const_tex);
	picker_shader.setInt("constant_stack", 1);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	float data[4];
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, &data);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, view_state.width, view_state.height);

	PickerResult result;
	result.value = glm::vec2(data[0], data[1]);
	result.coordinate = glm::vec2(data[2], data[3]);
	result.valid = true;

	return result;
}

void init_picker(unsigned int& picker_tex, unsigned int& picker_fbo) {
	glGenFramebuffers(1, &picker_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, picker_fbo);
	glGenTextures(1, &picker_tex);
	glBindTexture(GL_TEXTURE_2D, picker_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picker_tex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Picker Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
