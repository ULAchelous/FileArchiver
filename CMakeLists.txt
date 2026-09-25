cmake_minimum_required(VERSION 3.16)

project(FileArchiver
    VERSION 0.1.0
    DESCRIPTION "File archiving utility"
    LANGUAGES CXX
)

# ---- 全局设置 ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- 模块路径（优先使用本项目的 cmake/ 目录）----
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

# ---- 输出目录 ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---- 自动收集源文件 ----
# CONFIGURE_DEPENDS：新增/删除源文件后重新构建时会自动触发 cmake 重新配置
# 约定：src/ 下所有 .cpp 都编入核心库，main.cpp 单独排除（属于可执行入口）
file(GLOB_RECURSE FILEARCHIVER_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER FILEARCHIVER_CORE_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

if(FILEARCHIVER_CORE_SOURCES)
    message(STATUS "Core library sources:")
    foreach(f IN LISTS FILEARCHIVER_CORE_SOURCES)
        message(STATUS "  ${f}")
    endforeach()
endif()

# ---- 第三方库：yaml-cpp（源码内嵌，三平台通用）----
# yaml-cpp 0.8.0 的 cmake_minimum_required 过老，CMake 4.x 需要此开关
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(libs/yaml-cpp EXCLUDE_FROM_ALL)

# ---- 主库 ----
add_library(filearchiver_core
    ${FILEARCHIVER_CORE_SOURCES}
)
add_library(filearchiver::core ALIAS filearchiver_core)

target_include_directories(filearchiver_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(filearchiver_core PUBLIC yaml-cpp)

# ---- 可执行文件 ----
add_executable(filearchiver
    src/main.cpp
)
target_link_libraries(filearchiver PRIVATE filearchiver::core)
set_target_properties(filearchiver PROPERTIES OUTPUT_NAME FileArchiver)

# ---- 测试 ----
include(CTest)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- 安装规则 ----
install(TARGETS filearchiver filearchiver_core
    EXPORT FileArchiverTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ---- 版本信息输出 ----
message(STATUS "FileArchiver version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
