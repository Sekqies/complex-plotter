set(SOURCE_FILES 
    "graphics/graphics.cpp"
    "graphics/graphics.h"
 "types/types.h" "parser/tokenizer.h" "parser/checker.h" "parser/rpn.h" "parser/parser.h" "parser/checker.cpp" "parser/parser.cpp" "parser/rpn.cpp" "parser/tokenizer.cpp" "parser/simplifier.h" "parser/simplifier.cpp" "transformer/constant.h" "transformer/constant.cpp" "preprocessor/preprocessor.h" "preprocessor/string_builder.h"  "transformer/operator.h" "transformer/operator.cpp" "transformer/transformer.h" "transformer/transformer.cpp" "preprocessor/string_builder.cpp" "preprocessor/preprocessor.cpp" "interactions/interactions.h" "interactions/interactions.cpp" "parser/validator.cpp" "parser/validator.h" "graphics/ui_init.h" "graphics/ui_init.cpp" "graphics/ui.h" "graphics/ui.cpp"  "types/type_mapper.h"  "types/type_mapper.cpp" "compiler/compiler_shader.h" "compiler/compiler_shader.cpp" "transformer/tree.h" "transformer/tree.cpp" "higher_order/derivative.h" "higher_order/higher_order.h" "higher_order/derivative.cpp" "higher_order/higher_order.cpp" "graphics/3d/mesh.h" "graphics/3d/mesh.cpp" "graphics/3d/camera_state.h" "graphics/3d/camera_state.cpp" "graphics/picker.cpp")
set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(HEADER_BUILD "${CMAKE_CURRENT_BINARY_DIR}/shaders/embedded_shaders.h")
set(HEADER_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/shaders/embedded_shaders.h")

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl"
)

add_custom_command(
    OUTPUT ${HEADER_BUILD} ${HEADER_SRC}
    
    COMMAND ${CMAKE_COMMAND} 
        -DSHADER_SOURCE_DIR="${SHADERS_DIR}"
        -DDEST_BUILD="${HEADER_BUILD}"
        -DDEST_SRC="${HEADER_SRC}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_shaders.cmake"
        
    DEPENDS ${SHADER_FILES}
    COMMENT "[Custom] Generating embedded_shaders.h in build and src folders..."
)


add_executable(complex-plotter ${SOURCE_FILES} ${HEADER_SRC} "main.cpp")
find_package(OpenGL REQUIRED)

target_link_libraries(complex-plotter PRIVATE 
    glad
    glfw        
    glm::glm    
    OpenGL::GL
    learnopengl
    imgui
)
add_custom_target(copy-additional-folders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "${CMAKE_CURRENT_BINARY_DIR}/shaders" 
    COMMENT "[Custom] Copying additional folders to build directory..."
)

add_dependencies(complex-plotter copy-additional-folders)
target_include_directories(complex-plotter PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ".")