cmake_minimum_required(VERSION 3.10)

project(colgm VERSION 0.1)

message("CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")

# -std=c++17 -Wshadow -Wall
if (MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set compile option
if (NOT MSVC)
    add_compile_options(-fPIC)
endif()
add_compile_options(-I ${CMAKE_SOURCE_DIR})
if (APPLE)
    add_compile_options(-mmacosx-version-min=10.15)
endif()

# build colgm used object
set(COLGM_AST
    ${CMAKE_SOURCE_DIR}/ast/ast.cpp
    ${CMAKE_SOURCE_DIR}/ast/decl.cpp
    ${CMAKE_SOURCE_DIR}/ast/delete_disabled_node.cpp
    ${CMAKE_SOURCE_DIR}/ast/dumper.cpp
    ${CMAKE_SOURCE_DIR}/ast/expr.cpp
    ${CMAKE_SOURCE_DIR}/ast/replace_defer.cpp
    ${CMAKE_SOURCE_DIR}/ast/stmt.cpp
    ${CMAKE_SOURCE_DIR}/ast/visitor.cpp)
set(COLGM_MIR
    ${CMAKE_SOURCE_DIR}/mir/add_default_func.cpp
    ${CMAKE_SOURCE_DIR}/mir/adjust_va_arg_func.cpp
    ${CMAKE_SOURCE_DIR}/mir/ast2mir.cpp
    ${CMAKE_SOURCE_DIR}/mir/mir.cpp
    ${CMAKE_SOURCE_DIR}/mir/pass_manager.cpp
    ${CMAKE_SOURCE_DIR}/mir/type_cast_number_pass.cpp
    ${CMAKE_SOURCE_DIR}/mir/visitor.cpp)
set(COLGM_SIR
    ${CMAKE_SOURCE_DIR}/sir/adjust_va_arg_func.cpp
    ${CMAKE_SOURCE_DIR}/sir/context.cpp
    ${CMAKE_SOURCE_DIR}/sir/control_flow.cpp
    ${CMAKE_SOURCE_DIR}/sir/debug_info.cpp
    ${CMAKE_SOURCE_DIR}/sir/detect_redef_extern.cpp
    ${CMAKE_SOURCE_DIR}/sir/mir2sir.cpp
    ${CMAKE_SOURCE_DIR}/sir/pass_manager.cpp
    ${CMAKE_SOURCE_DIR}/sir/primitive_size_opt.cpp
    ${CMAKE_SOURCE_DIR}/sir/replace_ptr_call.cpp
    ${CMAKE_SOURCE_DIR}/sir/simplify_cfg.cpp
    ${CMAKE_SOURCE_DIR}/sir/sir.cpp)
set(COLGM_PACKAGE
    ${CMAKE_SOURCE_DIR}/package/package.cpp)
set(COLGM_SEMA
    ${CMAKE_SOURCE_DIR}/sema/context.cpp
    ${CMAKE_SOURCE_DIR}/sema/func.cpp
    ${CMAKE_SOURCE_DIR}/sema/regist_pass.cpp
    ${CMAKE_SOURCE_DIR}/sema/semantic.cpp
    ${CMAKE_SOURCE_DIR}/sema/struct.cpp
    ${CMAKE_SOURCE_DIR}/sema/tagged_union.cpp
    ${CMAKE_SOURCE_DIR}/sema/type.cpp
    ${CMAKE_SOURCE_DIR}/sema/type_resolver.cpp)
set(COLGM_OBJECT
    ${CMAKE_SOURCE_DIR}/lexer.cpp
    ${CMAKE_SOURCE_DIR}/misc.cpp
    ${CMAKE_SOURCE_DIR}/parse/parse.cpp
    ${CMAKE_SOURCE_DIR}/report.cpp)

add_library(colgm-ast STATIC ${COLGM_AST})
target_include_directories(colgm-ast PRIVATE ${CMAKE_SOURCE_DIR})
add_library(colgm-mir STATIC ${COLGM_MIR})
target_include_directories(colgm-mir PRIVATE ${CMAKE_SOURCE_DIR})
add_library(colgm-sir STATIC ${COLGM_SIR})
target_include_directories(colgm-sir PRIVATE ${CMAKE_SOURCE_DIR})
add_library(colgm-package STATIC ${COLGM_PACKAGE})
target_include_directories(colgm-package PRIVATE ${CMAKE_SOURCE_DIR})
add_library(colgm-sema STATIC ${COLGM_SEMA})
target_include_directories(colgm-sema PRIVATE ${CMAKE_SOURCE_DIR})
add_library(colgm-object STATIC ${COLGM_OBJECT})
target_include_directories(colgm-object PRIVATE ${CMAKE_SOURCE_DIR})

# build colgm
add_executable(colgm ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(colgm
    colgm-object
    colgm-ast
    colgm-sir
    colgm-mir
    colgm-package
    colgm-sema)
