cmake_minimum_required(VERSION 3.16)
project(LanChat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)                                                                             




# 查找 Qt（改进的逻辑）
find_package(Qt6 COMPONENTS Core Widgets Sql WebSockets Network QUIET)
if (Qt6_FOUND)
    message(STATUS "Found Qt6: ${Qt6_DIR}")
    set(QT_VERSION_MAJOR 6)
else()
    message(STATUS "Qt6 not found, trying Qt5...")
    find_package(Qt5 COMPONENTS Core Widgets Sql WebSockets Network QUIET)
    if (Qt5_FOUND)
        message(STATUS "Found Qt5: ${Qt5_DIR}")
        set(QT_VERSION_MAJOR 5)
    else()
        message(FATAL_ERROR "Neither Qt6 nor Qt5 found!\n"
                            "Please set CMAKE_PREFIX_PATH to your Qt installation directory.\n"
                            "Example: cmake -DCMAKE_PREFIX_PATH=D:/ProgramData/qt/6.5.3/msvc2019_64 -S . -B build")
    endif()
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# --- 保留 feature/mty/db 定义的 UI 文件变量 ---
set(UI_FILES src/ui/main_window/main_window.ui) 

# 源文件 (合并了 dev 和 feature/mty/db 中的所有文件)
set(SOURCES
    # Main
    src/main.cpp
    
    # core - 新增的多线程架构
    src/core/base_controller.cpp
    src/core/base_worker.cpp
    src/core/network_controller.cpp
    src/core/network_worker.cpp
    src/core/dblogic_controller.cpp
    src/core/dblogic_worker.cpp
    src/core/app_context.cpp
    
    # utils (来自 feature/mty/db)
    src/utils/logger.cpp
    src/utils/config.cpp
    src/utils/db_manager.cpp
    src/utils/datebase.cpp
    src/utils/password_util.cpp

    # model (来自 feature/mty/db)
    src/model/message_dao.cpp

    # service (来自 feature/mty/db)
    src/service/chat_service.cpp
    src/service/auth_service.cpp
    
    # ui - main_window (来自 feature/mty/db 和 dev)
    src/ui/main_window/main_window.cpp
    src/ui/db_qwidget/DbQWidget.cpp
    src/ui/setting/settingdialog.cpp
    # ui - login (S3: 登录窗口)
    src/ui/login/login_window.cpp
    src/ui/login/register_window.cpp
    
    # ui - dev 新增的 UI 文件
    src/ui/main_window/main_window.h
    # 使用 chatpage 版本的 ChatWindow 作为主聊天界面
    src/ui/chatpage/chatwindow.cpp
    src/ui/chatpage/chatwindow.h
    src/ui/main_window/ContactList.cpp
    src/ui/main_window/ContactList.h
    src/ui/main_window/MessageList.cpp
    src/ui/main_window/MessageList.h
    src/ui/main_window/ContactItem.cpp
    src/ui/main_window/ContactItem.h
    src/ui/main_window/add_friend_dialog.cpp
    src/ui/main_window/add_friend_dialog.h
    src/ui/main_window/friend_request_item.cpp
    src/ui/main_window/friend_request_item.h
    src/ui/network_error/network_error_dialog.cpp
    # chatpage 依赖的通用聊天控件
    src/ui/chatpage/nchatview.cpp
    src/ui/chatpage/nchatview.h
    src/ui/chatpage/nchatmessageitem.cpp
    src/ui/chatpage/nchatmessageitem.h
    src/ui/chatpage/pixmaputils.cpp
    src/ui/chatpage/pixmaputils.h
    # network (来自 feature/mty/db)
    src/common/types.h
    src/network/socket_client.cpp
    src/network/socket_client.h
    src/network/socket_server.cpp
    src/network/socket_server.h

    src/ui/main_window/ChatInputWidget.h
    src/ui/main_window/ChatInputWidget.cpp
    src/ui/main_window/ChatPage.cpp
    src/ui/main_window/ChatPage.h
    src/ui/main_window/ChatTitle.cpp
    src/ui/main_window/ChatTitle.h
    src/ui/main_window/DataLoader.h
    src/ui/main_window/DataLoader.cpp
    src/ui/main_window/Message.h
    src/ui/main_window/NChatMessageItem.h
    src/ui/main_window/NChatMessageItem.cpp
    src/ui/main_window/NChatView.cpp
    src/ui/main_window/NChatView.h
    src/ui/main_window/PixmapUtils.h
    src/ui/main_window/PixmapUtils.cpp
    src/ui/main_window/SessionInfo.cpp
    src/ui/main_window/SessionInfo.h
    src/ui/main_window/SessionList.cpp
    src/ui/main_window/SessionList.h
    src/ui/main_window/SessionListItem.h
    src/ui/main_window/SessionListItem.cpp





  
    
 
    src/ui/personinfo/ProfileEditDialog.cpp
    src/ui/personinfo/ProfileViewDialog.cpp
    # UI 文件变量 (来自 feature/mty/db)
    ${UI_FILES} # 将 .ui 文件加入到目标源中，启用 AUTOUIC 生成 ui_main_window.h
)

# 创建可执行文件
if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(LanChat ${SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/assets/resources.qrc"
    )
    target_link_libraries(LanChat PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::WebSockets
        Qt6::Network
    )
else()
    add_executable(LanChat ${SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/assets/resources.qrc"
    )
    target_link_libraries(LanChat
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
        Qt5::WebSockets
        Qt5::Network
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
