cmake_minimum_required(VERSION 3.10)

project(colgm VERSION 10.1)

message("CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")

# -std=c++17 -Wshadow -Wall
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wuninitialized")

# set compile option
add_compile_options(-fPIC)
add_compile_options(-I ${CMAKE_SOURCE_DIR}/src)
if (APPLE)
    add_compile_options(-mmacosx-version-min=10.15)
endif()

# build colgm used object
set(COLGM_AST
    ${CMAKE_SOURCE_DIR}/src/ast/ast.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/decl.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/dumper.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/expr.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/stmt.cpp
    ${CMAKE_SOURCE_DIR}/src/ast/visitor.cpp)
set(COLGM_SIR
    ${CMAKE_SOURCE_DIR}/src/sir/context.cpp
    ${CMAKE_SOURCE_DIR}/src/sir/delete_useless_label.cpp
    ${CMAKE_SOURCE_DIR}/src/sir/pass_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/sir/remove_alloca.cpp
    ${CMAKE_SOURCE_DIR}/src/sir/sir.cpp)
set(COLGM_MIR
    ${CMAKE_SOURCE_DIR}/src/mir/add_default_func.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/ast2mir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/mir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/mir2sir.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/pass_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/type_cast_number_pass.cpp
    ${CMAKE_SOURCE_DIR}/src/mir/visitor.cpp)
set(COLGM_PACKAGE
    ${CMAKE_SOURCE_DIR}/src/package/package.cpp)
set(COLGM_SEMA
    ${CMAKE_SOURCE_DIR}/src/sema/context.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/func.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/semantic.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/struct.cpp
    ${CMAKE_SOURCE_DIR}/src/sema/symbol.cpp)
set(COLGM_OBJECT
    ${CMAKE_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/misc.cpp
    ${CMAKE_SOURCE_DIR}/src/parse.cpp
    ${CMAKE_SOURCE_DIR}/src/report.cpp)

add_library(colgm-ast STATIC ${COLGM_AST})
target_include_directories(colgm-ast PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_library(colgm-sir STATIC ${COLGM_SIR})
target_include_directories(colgm-sir PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_library(colgm-mir STATIC ${COLGM_MIR})
target_include_directories(colgm-mir PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_library(colgm-package STATIC ${COLGM_PACKAGE})
target_include_directories(colgm-package PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_library(colgm-sema STATIC ${COLGM_SEMA})
target_include_directories(colgm-sema PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_library(colgm-object STATIC ${COLGM_OBJECT})
target_include_directories(colgm-object PRIVATE ${CMAKE_SOURCE_DIR}/src)

# build colgm
add_executable(colgm ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(colgm colgm-object colgm-ast colgm-sir colgm-mir colgm-package colgm-sema)
