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
find_package(Qt6 COMPONENTS Core Widgets Network Sql QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Network Sql REQUIRED)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# UI 文件目录
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/ui/main_window)

# 源文件
set(SOURCES
    src/main.cpp
    src/ui/main_window/main_window.cpp
    src/utils/logger.cpp
    src/utils/config.cpp
    src/network/socket_client.cpp
    src/service/chat_service.cpp
)

set(HEADERS
    src/ui/main_window/main_window.h
    src/common/global.h
    src/common/types.h
    src/utils/logger.h
    src/utils/config.h
    src/network/socket_client.h
    src/service/chat_service.h
)

set(UI_FILES
    src/ui/main_window/main_window.ui
)

set(RESOURCES
    src/ui/assets/resources.qrc
)

# 创建可执行文件
if (Qt6_FOUND)
    qt_add_executable(LanChat
        ${SOURCES}
        ${HEADERS}
        ${UI_FILES}
        ${RESOURCES}
    )
    target_link_libraries(LanChat PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
    )
else()
    add_executable(LanChat
        ${SOURCES}
        ${HEADERS}
        ${UI_FILES}
        ${RESOURCES}
    )
    target_link_libraries(LanChat
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        Qt5::Sql
    )
endif()

# 设置 Windows 子系统（避免控制台窗口）
if(WIN32)
    set_target_properties(LanChat PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

# 安装规则
install(TARGETS LanChat
    RUNTIME DESTINATION bin
)
