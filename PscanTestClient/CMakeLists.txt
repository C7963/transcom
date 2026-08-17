cmake_minimum_required(VERSION 3.16)
project(PscanTestClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Windows平台下使用多线程编译
if(MSVC)
    add_compile_options(/MP)
endif()

# 查找Qt - 优先查找Qt6，找不到则查找Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)
endif()

# 源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
    SpectrumWidget.cpp
)

set(HEADERS
    MainWindow.h
    SpectrumWidget.h
    PscanDriver.h
)

# 添加可执行文件
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# 包含BSPDriver头文件目录（仅用于PscanDefs.h类型定义）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../BSPDriver
)

# 链接Qt库和系统库（用于LoadLibrary/GetProcAddress）
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Widgets)

# Windows特定设置 - 使用Unicode
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
endif()
