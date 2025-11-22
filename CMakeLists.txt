cmake_minimum_required(VERSION 3.16)
project(LanChat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 查找 Qt
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 源文件
set(SOURCES
    src/main.cpp
)

# 创建可执行文件
if (Qt6_FOUND)
    qt_add_executable(LanChat ${SOURCES})
    target_link_libraries(LanChat PRIVATE
        Qt6::Core
        Qt6::Widgets
    )
else()
    add_executable(LanChat ${SOURCES})
    target_link_libraries(LanChat
        Qt5::Core
        Qt5::Widgets
    )
endif()

# 设置 Windows 子系统（避免控制台窗口）
if(WIN32)
    set_target_properties(LanChat PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

# 设置编译器警告级别
if(MSVC)
    target_compile_options(LanChat PRIVATE /W3)
else()
    target_compile_options(LanChat PRIVATE -Wall -Wextra)
endif()

# 安装规则
install(TARGETS LanChat
    RUNTIME DESTINATION bin
)
