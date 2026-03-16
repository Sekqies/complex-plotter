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
    
    string(LENGTH "${SHADER_TEXT}" FULL_LENGTH)
    
    set(CHUNK_SIZE 8000)
    set(CURRENT_OFFSET 0)
    set(CHUNKS "")

    while(CURRENT_OFFSET LESS FULL_LENGTH)
        math(EXPR REMAINING "${FULL_LENGTH} - ${CURRENT_OFFSET}")
        if(REMAINING GREATER CHUNK_SIZE)
            set(STR_LIMIT ${CHUNK_SIZE})
        else()
            set(STR_LIMIT ${REMAINING})
        endif()

        string(SUBSTRING "${SHADER_TEXT}" ${CURRENT_OFFSET} ${STR_LIMIT} CHUNK_DATA)
        string(APPEND CHUNKS "R\"shdr(${CHUNK_DATA})shdr\" ")

        math(EXPR CURRENT_OFFSET "${CURRENT_OFFSET} + ${STR_LIMIT}")
    endwhile()

    string(APPEND FILE_CONTENT "inline std::string SRC_${VAR_NAME} = \n    ${CHUNKS};\n\n")
endforeach()

file(WRITE "${DEST_BUILD}" "${FILE_CONTENT}")
file(WRITE "${DEST_SRC}" "${FILE_CONTENT}")