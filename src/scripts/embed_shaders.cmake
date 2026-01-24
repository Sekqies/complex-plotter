file(GLOB SHADER_FILES 
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.geom"
    "${SHADER_SOURCE_DIR}/*.glsl")

set(FILE_CONTENT "#pragma once\n#include <string>\n\n")

foreach(FILE ${SHADER_FILES})
    get_filename_component(FILENAME ${FILE} NAME)
    string(REPLACE "." "_" VAR_NAME ${FILENAME})
    string(TOUPPER ${VAR_NAME} VAR_NAME)
    
    file(READ ${FILE} SHADER_TEXT)
    
    string(APPEND FILE_CONTENT "inline std::string SRC_${VAR_NAME} = R\"(${SHADER_TEXT})\";\n\n")
endforeach()

file(WRITE "${DEST_BUILD}" "${FILE_CONTENT}")
file(WRITE "${DEST_SRC}" "${FILE_CONTENT}")