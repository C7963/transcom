cmake_minimum_required(VERSION 3.16)
project(TransComApi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义导出宏
add_definitions(-DBSPDRIVER_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

# boost头文件路径
set(BOOST_INCLUDE_DIR "C:/boost_1_91_0/boost_1_91_0")

# 源文件（核心扫描驱动）
set(SOURCES
    PscanApi.cpp
    Pscan.cpp
    PscanOrder.cpp
    SpectrumSweepFft.cpp
    SpectrumSweep.cpp
    Sweep.cpp
    SweepLogic.cpp
    XillyFile.cpp
    Device_MEM32.cpp
    CommonManager.cpp
    Global.cpp
    Device_Address.cpp
    ConvertHelper.cpp
)

set(HEADERS
    PscanApi.h
    PscanDefs.h
    Pscan.h
    PscanOrder.h
    SpectrumSweepFft.h
    SpectrumSweep.h
    IPscanDataSource.h
    IPscanDataSink.h
)

# 创建DLL
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/../packages/pugixml.1.15.0/build/native/include"
)

# Windows特定设置
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    BSPDRIVER_EXPORTS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# 使用UTF-8编码编译，避免中文注释导致的解析错误
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

# 禁用预编译头
set_target_properties(${PROJECT_NAME} PROPERTIES
    DISABLE_PRECOMPILE_HEADERS ON
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)
