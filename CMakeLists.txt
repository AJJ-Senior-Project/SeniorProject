cmake_minimum_required(VERSION 3.16)

project(StreamNDI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only use Qt6, ensure it's required.
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

add_executable(NDIQtBackend ${PROJECT_SOURCES}
    ndi_sender.h
    ndi_sender.cpp)

# Link Qt6 libraries correctly
target_link_libraries(NDIQtBackend Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

# Change this to your NDI SDK path
set(NDI_SDK_PATH "C:/Program Files/NDI/NDI 6 SDK")
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

qt_add_executable(StreamNDI
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# Link Qt Multimedia and Widgets components properly for StreamNDI
target_link_libraries(StreamNDI PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

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

qt_finalize_executable(StreamNDI)
