# include some defines automatically made by qpm
include(qpm_defines.cmake)

cmake_minimum_required(VERSION 3.22)
project(${COMPILE_ID})
include(${EXTERN_DIR}/includes/kaleb/shared/cmake/assets.cmake)

# c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

# Enable link time optimization
# In my experience, this can be highly unstable but it nets a huge size optimization and likely performance
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# define that stores the actual source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# stop symbol leaking
add_link_options(-Wl,--exclude-libs,ALL)
# compile options used
add_compile_options(-frtti -fPIE -fPIC -fexceptions -fdeclspec -fvisibility=hidden)

# get git info
execute_process(COMMAND git config user.name OUTPUT_VARIABLE GIT_USER)
execute_process(COMMAND git branch --show-current OUTPUT_VARIABLE GIT_BRANCH)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_COMMIT)
execute_process(COMMAND git diff-index --quiet HEAD RESULT_VARIABLE GIT_MODIFIED)

string(STRIP "${GIT_USER}" GIT_USER)
string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
string(STRIP "${GIT_COMMIT}" GIT_COMMIT)
string(STRIP "${GIT_MODIFIED}" GIT_MODIFIED)

message(STATUS "GIT_USER: ${GIT_USER}")
message(STATUS "GIT_BRANCH: ${GIT_BRANCH}")
message(STATUS "GIT_COMMIT: 0x${GIT_COMMIT}")
message(STATUS "GIT_MODIFIED: ${GIT_MODIFIED}")

# Check for file presence and read current contents
set(GIT_INFO_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/git_info.h")
if(EXISTS "${GIT_INFO_H_PATH}")
    file(READ "${GIT_INFO_H_PATH}" GIT_INFO_H_CURRENT)
else()
    set(GIT_INFO_H_CURRENT "")
endif()

# Define new git info content
set(GIT_INFO_H "#pragma once
#define GIT_USER \"${GIT_USER}\"
#define GIT_BRANCH \"${GIT_BRANCH}\"
#define GIT_COMMIT 0x${GIT_COMMIT}
#define GIT_MODIFIED ${GIT_MODIFIED}
")

# Write git info to file if the contents have changed
if(NOT "${GIT_INFO_H}" STREQUAL "${GIT_INFO_H_CURRENT}")
    file(WRITE "${GIT_INFO_H_PATH}" "${GIT_INFO_H}")
endif()

# compile definitions used
add_compile_definitions(VERSION=\"${MOD_VERSION}\")
add_compile_definitions(MOD_ID=\"${MOD_ID}\")
add_compile_definitions(MAKE_DOCS=${MAKE_DOCS})
add_compile_definitions(UNITY_2021)
add_compile_definitions(CORDL_RUNTIME_FIELD_NULL_CHECKS)

# recursively get all src files
RECURSE_FILES(cpp_file_list ${SOURCE_DIR}/*.cpp)
RECURSE_FILES(c_file_list ${SOURCE_DIR}/*.c)

RECURSE_FILES(inline_hook_c ${EXTERN_DIR}/includes/beatsaber-hook/shared/inline-hook/*.c)
RECURSE_FILES(inline_hook_cpp ${EXTERN_DIR}/includes/beatsaber-hook/shared/inline-hook/*.cpp)

# add all src files to compile
add_library(
    ${COMPILE_ID}
    SHARED
    ${cpp_file_list}
    ${c_file_list}
    ${inline_hook_c}
    ${inline_hook_cpp}
    "${CMAKE_CURRENT_SOURCE_DIR}/include/submodules/css-color-parser-cpp/csscolorparser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/submodules/EasyGifReader/EasyGifReader.cpp"
)

# add src dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${SOURCE_DIR})
# add include dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${INCLUDE_DIR})
# add shared dir as include dir
target_include_directories(${COMPILE_ID} PUBLIC ${SHARED_DIR})
# codegen includes
target_include_directories(${COMPILE_ID} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/submodules/css-color-parser-cpp/")
target_include_directories(${COMPILE_ID} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/submodules/EasyGifReader/")
target_include_directories(${COMPILE_ID} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/submodules/cppcodec/")
target_include_directories(${COMPILE_ID} PRIVATE ${EXTERN_DIR}/includes/gif-lib/shared)

add_assets(${COMPILE_ID}-assets STATIC ${CMAKE_CURRENT_LIST_DIR}/assets ${INCLUDE_DIR}/assets.hpp)
target_link_libraries(${COMPILE_ID} PRIVATE -llog -lz ${COMPILE_ID}-assets)

# add extern stuff like libs and other includes
include(extern.cmake)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_STRIP} -g -S -d --strip-all
            "lib${COMPILE_ID}.so" -o "stripped_lib${COMPILE_ID}.so"
            COMMENT "Strip debug symbols done on final binary.")

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory debug
        COMMENT "Make directory for debug symbols"
        )

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename lib${COMPILE_ID}.so debug/lib${COMPILE_ID}.so
            COMMENT "Rename the lib to debug_ since it has debug symbols"
            )

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename stripped_lib${COMPILE_ID}.so lib${COMPILE_ID}.so
            COMMENT "Rename the stripped lib to regular"
            )

foreach(so_file ${so_list})
    cmake_path(GET so_file FILENAME file)

    add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${so_file} debug/${file}
        COMMENT "Copy so files for ndk stack"
    )

    add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_STRIP} -g -S -d --strip-all ${so_file} -o ${file}
            COMMENT "Strip debug symbols from the dependencies")
endforeach()

foreach(a_file ${a_list})
cmake_path(GET a_file FILENAME file)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${a_file} debug/${file}
    COMMENT "Copy a files for ndk stack")
endforeach()
