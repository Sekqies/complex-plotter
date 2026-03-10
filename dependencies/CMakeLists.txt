include(FetchContent)


set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        docking  
)

if(NOT EMSCRIPTEN)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.3.8
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)


if(NOT EMSCRIPTEN)
    add_library(glad STATIC glad/src/glad.c)
    target_include_directories(glad PUBLIC glad/include)
endif()


add_library(learnopengl
    "learnopengl/src/shader.cpp"
    "learnopengl/include/shader/shader.h"
    "learnopengl/include/stb_image/stb_image.h" 
    "learnopengl/src/stb_image.cpp" "learnopengl/include/camera/camera.h" "learnopengl/src/camera.cpp" "learnopengl/include/VAO/VAO.h" "learnopengl/include/VBO/VBO.h" "learnopengl/src/VAO.cpp" "learnopengl/src/VBO.cpp" "learnopengl/include/texture/texture.h" "learnopengl/src/texture.cpp")

target_include_directories(learnopengl PUBLIC learnopengl/include)
if(EMSCRIPTEN)
    target_link_libraries(learnopengl PUBLIC glm::glm)
else()
    target_link_libraries(learnopengl PUBLIC glad glm::glm OpenGL::GL)
endif()

if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
endif()

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
 )

target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
)

if(EMSCRIPTEN)
    target_compile_options(imgui PRIVATE "-s" "USE_GLFW=3")
else()
    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
    target_link_libraries(learnopengl PUBLIC OpenGL::GL)
endif()
