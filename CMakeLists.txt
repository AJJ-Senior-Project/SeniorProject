cmake_minimum_required(VERSION 3.16)

project(StreamNDI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

# Set the project source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ndi_sender.h
    ndi_sender.cpp
)

# Add the executable for StreamNDI
qt_add_executable(StreamNDI
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    resource.qrc
    ndi_sender.h ndi_sender.cpp
    ndi_receiver.h ndi_receiver.cpp
    discoveryworker.h discoveryworker.cpp
    receiverworker.h receiverworker.cpp
    senderworker.h
    senderworker.cpp

)

# Link Qt libraries to StreamNDI
target_link_libraries(StreamNDI PRIVATE ${NDI_LIB} Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets Gdi32)

# Change this to your NDI SDK path
set(NDI_SDK_PATH "C:/Program Files/NDI/NDI 6 SDK")
include_directories(${NDI_SDK_PATH}/Include)  # Ensure this is the correct include path

# Add the NDI library
set(NDI_LIB_NAME "Processing.NDI.Lib.x64")
find_library(NDI_LIB NAMES ${NDI_LIB_NAME} Processing.NDI.Lib PATHS ${NDI_SDK_PATH}/Lib/x64)

# Ensure the NDI library is found, otherwise error out
if(NDI_LIB)
    target_link_libraries(StreamNDI PRIVATE ${NDI_LIB})
else()
    message(FATAL_ERROR "NDI library not found at ${NDI_SDK_PATH}/Lib/x64")
endif()

# Copy the NDI .dll file to the output directory after build
add_custom_command(TARGET StreamNDI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${NDI_SDK_PATH}/Bin/x64/Processing.NDI.Lib.x64.dll"  # Source location of the .dll
    $<TARGET_FILE_DIR:StreamNDI>)  # Destination: executable directory

# Set project properties for Windows and macOS
set_target_properties(StreamNDI PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Installation setup
include(GNUInstallDirs)
install(TARGETS StreamNDI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalize the Qt executable
qt_finalize_executable(StreamNDI)
