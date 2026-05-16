cmake_minimum_required(VERSION 3.16)
project(MinCutTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
endif()

add_executable(MinCutTool
    main.cpp
    mainwindow.cpp
    graphwidget.cpp
    mincut.h
    mainwindow.h
    graphwidget.h
)

if(Qt6_FOUND)
    target_link_libraries(MinCutTool Qt6::Core Qt6::Gui Qt6::Widgets)
else()
    target_link_libraries(MinCutTool Qt5::Core Qt5::Gui Qt5::Widgets)
endif()
