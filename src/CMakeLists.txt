set(SOURCE_FILES 
    "graphics/graphics.cpp"
    "graphics/graphics.h"
 "parser/types.h" "parser/tokenizer.h" "parser/checker.h" "parser/rpn.h" "parser/parser.h" "parser/checker.cpp" "parser/parser.cpp" "parser/rpn.cpp" "parser/tokenizer.cpp" "parser/simplifier.h" "parser/simplifier.cpp" "transformer/constant.h" "transformer/constant.cpp" "preprocessor/preprocessor.h" "preprocessor/string_builder.h")

add_executable(complex-plotter ${SOURCE_FILES} "main.cpp")
find_package(OpenGL REQUIRED)

target_link_libraries(complex-plotter PRIVATE 
    glad
    glfw        
    glm::glm    
    OpenGL::GL
    learnopengl
)
add_custom_target(copy-additional-folders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "${CMAKE_CURRENT_BINARY_DIR}/shaders" 
    COMMENT "Copying additional folders to build directory..."
)

add_dependencies(complex-plotter copy-additional-folders)
target_include_directories(complex-plotter PRIVATE ".")