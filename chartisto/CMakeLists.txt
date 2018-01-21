find_package(jsoncpp)
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

target_link_libraries(chartisto
	gui
	jsoncpp_lib
)