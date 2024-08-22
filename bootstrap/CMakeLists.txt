cmake_minimum_required(VERSION 3.10)

project(colgm VERSION 10.1)

message("CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")

# -std=c++17 -Wshadow -Wall
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wshadow -Wall")

# set compile option
add_compile_options(-fPIC)
add_compile_options(-I ${CMAKE_SOURCE_DIR}/src)
if (APPLE)
    add_compile_options(-mmacosx-version-min=10.15)
endif()

# generate release executables
set(CMAKE_BUILD_TYPE "Release")

# build colgm used object
set(COLGM_OBJECT_SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/src/ast/ast.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/decl.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/dumper.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/expr.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/stmt.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/visitor.cpp
    ${CMAKE_SOURCE_DIR}/src/sir/context.cpp
    ${CMAKE_SOURCE_DIR}/src/sir/sir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/add_default_func.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/ast2mir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/mir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/mir2sir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/pass_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/type_cast_number_pass.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/visitor.cpp
    ${CMAKE_SOURCE_DIR}/src/package/package.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/context.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/semantic.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/symbol.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/misc.cpp
    ${CMAKE_SOURCE_DIR}/src/parse.cpp
    ${CMAKE_SOURCE_DIR}/src/report.cpp)
add_library(colgm-object STATIC ${COLGM_OBJECT_SOURCE_FILE})
target_include_directories(colgm-object PRIVATE ${CMAKE_SOURCE_DIR}/src)

# build colgm
add_executable(colgm ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(colgm colgm-object)
