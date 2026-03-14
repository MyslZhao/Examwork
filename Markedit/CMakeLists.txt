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
    I18N_TRANSLATED_LANGUAGES zh_CN en_US
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

target_sources(Markedit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/en_US.ts
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

get_target_property(lrelease_executable Qt${QT_VERSION_MAJOR}::lrelease IMPORTED_LOCATION)
if(NOT lrelease_executable OR NOT EXISTS ${lrelease_executable})
    message(FATAL_ERROR "lrelease not found. Please check your Qt installation.")
endif()

set(TRANSLATIONS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/translations)
file(MAKE_DIRECTORY ${TRANSLATIONS_OUTPUT_DIR})

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/en_US.ts
)

# 方便编辑.ts文件
set_source_files_properties(${TS_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND PROJECT_SOURCES ${TS_FILES})

foreach(ts_file ${TS_FILES})
    get_filename_component(ts_name ${ts_file} NAME_WE)
    set(qm_file ${TRANSLATIONS_OUTPUT_DIR}/${ts_name}.qm)

    add_custom_command(
        OUTPUT ${qm_file}
        COMMAND ${lrelease_executable} ${ts_file} -qm ${qm_file}
        DEPENDS ${ts_file}
        COMMENT "Generating ${qm_file}"
    )
    list(APPEND QM_FILES ${qm_file})
endforeach()

add_custom_target(translations ALL DEPENDS ${QM_FILES})

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR}/translations)


set(CPACK_PACKAGE_CONTACT "MyslZhao <vsetmysl@outlook.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6widgets6")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Markedit - A simple Markdown editor")
set(CPACK_PACKAGE_VENDOR "MyslZhao")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MyslZhao")

set(CPACK_GENERATOR "DEB")

include(CPack)
