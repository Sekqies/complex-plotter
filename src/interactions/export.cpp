#include <glad_include_guard.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>
#include <interactions/export.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void export_plot_to_png(const int width, const int height, const char* filename) {
	unsigned char* pixels = new unsigned char[width * height * 4];
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	stbi_set_flip_vertically_on_load(true);
	stbi_write_png(filename, width, height, 4, pixels, width * 4);
	delete[] pixels;
#ifdef __EMSCRIPTEN__
    EM_ASM_({
        const js_filename = UTF8ToString($0);

        const file_data = FS.readFile(js_filename);

        const blob = new Blob([file_data], { type: 'image/png' });
        const url = URL.createObjectURL(blob);
        const link = document.createElement('a');
        link.href = url;
        link.download = js_filename;

        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
            }, filename);
#endif

}
