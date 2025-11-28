cmake_minimum_required(VERSION 3.16)

project(LanChat_demo01 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        lanchat_demo01.cpp
        lanchat_demo01.h
        lanchat_demo01.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(LanChat_demo01
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        sidebar.h sidebar.cpp sidebar.ui
        chatwindow.h chatwindow.cpp chatwindow.ui
        chatpage.h chatpage.cpp chatpage.ui
        res.qrc

        sidebarbutton.h sidebarbutton.cpp

        sessionitemwidget.h sessionitemwidget.cpp
        icon.qrc
        nchatmessageitem.h nchatmessageitem.cpp
        pixmaputils.h pixmaputils.cpp
        nchatview.h nchatview.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET LanChat_demo01 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(LanChat_demo01 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(LanChat_demo01
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(LanChat_demo01 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.LanChat_demo01)
endif()
set_target_properties(LanChat_demo01 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS LanChat_demo01
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LanChat_demo01)
endif()
