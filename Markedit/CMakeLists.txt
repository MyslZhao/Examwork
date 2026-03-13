cmake_minimum_required(VERSION 3.16...4.1)
project(Markedit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS LinguistTools Widgets)

qt_standard_project_setup(
    I18N_TRANSLATED_LANGUAGES zh_CN en_US # 替换为你需要的语言
)

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
    statusBarHandle.cpp
    barNotesHandle.cpp
    barSettingsHandle.cpp
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

qt_add_translations(Markedit
    TS_FILES
        translations/zh_CN.ts
        translations/en_US.ts
)

include(CPack)

