cmake_minimum_required(VERSION 3.7.2)
set(CMAKE_CXX_STANDARD 14)
project(chartisto)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

pkg_check_modules(GTEST REQUIRED gtest>=1.7.0)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(test)
enable_testing()

find_package(Qt5Widgets)
find_package(Qt5LinguistTools)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TS chartisto_ru.ts)

set(SRC
	main.cpp
	config.cpp
	windowlist.cpp
)

add_library(chart
	chart.cpp
)

target_include_directories(chart INTERFACE .)

add_library(gui
	mainwindow.cpp
	saveas.cpp
	load.cpp
	macdform.cpp
	chartwidget.cpp
)
target_link_libraries (gui
        chart
	Qt5::Widgets
)

set(QRC chartisto.qrc)
configure_file(${QRC} ${QRC} COPYONLY)
qt5_add_translation(QM ${TS})
qt5_add_resources(QM_QRC ${CMAKE_CURRENT_BINARY_DIR}/${QRC})

set_property(SOURCE qrc_chartisto.cpp PROPERTY SKIP_AUTOGEN ON)

add_executable(chartisto ${SRC} ${QM_QRC})
target_link_libraries (chartisto gui ${JSONCPP_LIBRARIES})
