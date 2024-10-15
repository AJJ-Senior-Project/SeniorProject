cmake_minimum_required(VERSION 3.16)

project(StreamNDI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ndi_sender.cpp
    ndi_sender.h
)

add_executable(NDIQtBackend ${PROJECT_SOURCES}
    ndi_sender.h
    ndi_sender.cpp)
target_link_libraries(NDIQtBackend Qt6::Core Qt6::Gui Qt6::Widgets)

# Change this to your NDI SDK path
set(NDI_SDK_PATH "C:/Program Files/NDI/NDI6SDK")
include_directories(${NDI_SDK_PATH}/Include)  # Ensure this is the correct include path

# Specify the exact library name
set(NDI_LIB_NAME "Processing.NDI.Lib.x64")

# Look for the NDI library
find_library(NDI_LIB NAMES ${NDI_LIB_NAME} PATHS ${NDI_SDK_PATH}/Lib/x64)

if(NDI_LIB)
    target_link_libraries(NDIQtBackend ${NDI_LIB})
else()
    message(FATAL_ERROR "NDI library not found at ${NDI_SDK_PATH}/Lib/x64")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(StreamNDI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(StreamNDI SHARED ${PROJECT_SOURCES})
    else()
        add_executable(StreamNDI ${PROJECT_SOURCES})
    endif()
endif()

target_link_libraries(StreamNDI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER com.example.StreamNDI)
endif()

set_target_properties(StreamNDI PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS StreamNDI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(StreamNDI)
endif()
