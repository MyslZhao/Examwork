cmake_minimum_required(VERSION 3.16)
project(Markedit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt（优先 Qt6，降级 Qt5）
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 添加子目录：cmark 源码
add_subdirectory(third_party/cmark-0.31.2)

# 项目源文件
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    # 如果有翻译文件，可以添加：
    # Markedit_zh_CN.ts
)

# 创建可执行文件
add_executable(Markedit ${PROJECT_SOURCES})

# 链接 Qt 和 cmark 库
target_link_libraries(Markedit PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    cmark          # 由 add_subdirectory 提供的库目标
)

# 对于 Windows 设置 WIN32 子系统（避免控制台窗口）
if(WIN32)
    set_target_properties(Markedit PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# 安装规则（可选）
include(GNUInstallDirs)
install(TARGETS Markedit
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)
