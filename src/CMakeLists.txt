add_executable(math-transpiler "generate_cpp_complex_functions.cpp")

set(TRANSPILED_MAPPER_H "${CMAKE_CURRENT_SOURCE_DIR}/glsl_generated/generated_math_mapper.h")
set(MATH_GLSL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/shaders/complex_functions.frag")
set(TRANSPILER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/math_transpiler.py")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(
    OUTPUT ${TRANSPILED_MATH_H} ${TRANSPILED_MAPPER_H}
    COMMAND Python3::Interpreter ${TRANSPILER_SCRIPT} ${MATH_GLSL_SRC} "${CMAKE_CURRENT_SOURCE_DIR}/glsl_generated"
    DEPENDS ${MATH_GLSL_SRC} ${TRANSPILER_SCRIPT}
)

set(SOURCE_FILES 
    "graphics/graphics.cpp"
    "graphics/graphics.h"
 "types/types.h" "parser/tokenizer.h" "parser/checker.h" "parser/rpn.h" "parser/parser.h" "parser/checker.cpp" "parser/parser.cpp" "parser/rpn.cpp" "parser/tokenizer.cpp" "parser/simplifier.h" "parser/simplifier.cpp" "transformer/constant.h" "transformer/constant.cpp" "preprocessor/preprocessor.h" "preprocessor/string_builder.h"  "transformer/operator.h" "transformer/operator.cpp" "transformer/transformer.h" "transformer/transformer.cpp" "preprocessor/string_builder.cpp" "preprocessor/preprocessor.cpp" "interactions/interactions.h" "interactions/interactions.cpp" "parser/validator.cpp" "parser/validator.h" "graphics/ui_init.h" "graphics/ui_init.cpp" "graphics/ui.h" "graphics/ui.cpp"  "types/type_mapper.h"  "types/type_mapper.cpp" "compiler/compiler_shader.h" "compiler/compiler_shader.cpp" "transformer/tree.h" "transformer/tree.cpp" "higher_order/derivative.h" "higher_order/higher_order.h" "higher_order/derivative.cpp" "higher_order/higher_order.cpp" "graphics/3d/mesh.h" "graphics/3d/mesh.cpp" "graphics/3d/camera_state.h" "graphics/3d/camera_state.cpp" "graphics/picker.cpp" "glad_include_guard.h" "interactions/export.h" "preprocessor/transpiler.cpp" "preprocessor/transpiler.h" "glsl_generated/types.h" "interactions/export.cpp"
 "glsl_transpiled/glsl_big_number.h" "glsl_transpiled/glsl_types.h" "glsl_transpiled/glsl_constants.h" "glsl_transpiled/glsl_big_number.cpp" "high_precision/high_precision_constant.h" "high_precision/high_precision_constant.cpp" "cpu_drawing/cpu_render.h" "cpu_drawing/cpu_render.cpp")
set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(HEADER_BUILD "${CMAKE_CURRENT_BINARY_DIR}/shaders/embedded_shaders.h")
set(HEADER_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/shaders/embedded_shaders.h")

if(WIN32)
    list(APPEND SOURCE_FILES "app.rc")
endif()

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


add_executable(complex-plotter ${SOURCE_FILES} ${HEADER_SRC} "main.cpp" ${TRANSPILED_MAPPER_CPP})

if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
endif()

if(EMSCRIPTEN)
    target_link_libraries(complex-plotter PRIVATE 
        #glfw        
        glm::glm    
        learnopengl
        imgui
        boost_headers
    )
else()
    target_link_libraries(complex-plotter PRIVATE 
        glad
        glfw        
        glm::glm    
        OpenGL::GL
        learnopengl
        imgui
        boost_headers
    )
endif()

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    target_link_libraries(complex-plotter PRIVATE 
    pthread 
    dl
    ${X11_LIBRARIES}
    Xrandr Xinerama Xcursor Xi)
endif()

if(MSVC)
    set_target_properties(complex-plotter PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

add_custom_target(GenerateShaders DEPENDS ${HEADER_SRC})
add_dependencies(complex-plotter GenerateShaders)

target_include_directories(complex-plotter PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR} 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "."
)

if(EMSCRIPTEN)
    target_link_options(complex-plotter PRIVATE
    --bind
    -s USE_GLFW=3 
    -sUSE_WEBGL2=1
    -sMIN_WEBGL_VERSION=2
    -sMAX_WEBGL_VERSION=2
    -sALLOW_MEMORY_GROWTH=1
    -fexceptions
    -sASSERTIONS=1
    -sEXPORTED_RUNTIME_METHODS=[ccall]
    -pthread
    -sUSE_PTHREADS=1
    -sPTHREAD_POOL_SIZE=8
    )
    target_compile_options(complex-plotter PRIVATE "-s" "USE_GLFW=3" "-fexceptions")
    set_target_properties(complex-plotter PROPERTIES SUFFIX ".js")
    add_custom_command(TARGET complex-plotter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
        "${CMAKE_CURRENT_BINARY_DIR}/index.html"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/coi-serviceworker.min.js"
        "${CMAKE_CURRENT_BINARY_DIR}/coi-serviceworker.min.js"
        COMMENT "Copying custom index.html to build directory..."
    )
endif()

