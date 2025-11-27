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
find_package(Qt6 COMPONENTS Core Widgets Sql QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Sql REQUIRED)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# --- 保留 feature/mty/db 定义的 UI 文件变量 ---
set(UI_FILES src/ui/main_window/main_window.ui) 

# 源文件 (合并了 dev 和 feature/mty/db 中的所有文件)
set(SOURCES
    # Main
    src/main.cpp
    
    # utils (来自 feature/mty/db)
    src/utils/logger.cpp
    src/utils/config.cpp
    src/utils/db_manager.cpp
    src/utils/datebase.cpp

    # model (来自 feature/mty/db)
    src/model/message_dao.cpp

    # service (来自 feature/mty/db)
    src/service/chat_service.cpp
    
    # ui - main_window (来自 feature/mty/db 和 dev)
    src/ui/main_window/main_window.cpp
    src/ui/db_qwidget/DbQWidget.cpp
    
    # ui - dev 新增的 UI 文件
    src/ui/main_window/main_window.h
    src/ui/main_window/ChatWindow.cpp
    src/ui/main_window/ChatWindow.h
    src/ui/main_window/ContactList.cpp
    src/ui/main_window/ContactList.h
    src/ui/main_window/MessageList.cpp
    src/ui/main_window/MessageList.h
    src/ui/main_window/ContactItem.cpp
    src/ui/main_window/ContactItem.h
    
    # network (来自 feature/mty/db)
    # src/network/socket_client.cpp
    
    # UI 文件变量 (来自 feature/mty/db)
    ${UI_FILES} # 将 .ui 文件加入到目标源中，启用 AUTOUIC 生成 ui_main_window.h
)

# 创建可执行文件
if (Qt6_FOUND)
    qt_add_executable(LanChat ${SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/assets/resources.qrc"
    )
    target_link_libraries(LanChat PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
    )
else()
    add_executable(LanChat ${SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/assets/resources.qrc"
    )
    target_link_libraries(LanChat
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
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
    target_compile_options(LanChat PRIVATE
        /W3
        /utf-8
    )
else()
    target_compile_options(LanChat PRIVATE -Wall -Wextra)
endif()

# 安装规则
install(TARGETS LanChat
    RUNTIME DESTINATION bin
)
