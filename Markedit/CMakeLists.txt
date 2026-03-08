cmake_minimum_required(VERSION 3.16...4.1)
project(Markedit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 项目源文件
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    menuFileHandle.cpp
    highlighter.hpp
    highlighter.cpp
    menuEditHandle.cpp
    barOutlineHandle.cpp
    statusBar.cpp
    # Markedit_zh_CN.ts
)

add_executable(Markedit ${PROJECT_SOURCES}
)

target_link_libraries(Markedit PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(WIN32)
    set_target_properties(Markedit PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

include(GNUInstallDirs)
install(TARGETS Markedit
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)
