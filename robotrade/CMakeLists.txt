find_package(Boost REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

add_executable(robotrade
	robotrade.cpp
)

add_executable(quotetool
	quoteTool.cpp
)

add_library(robotradeutils
	tripleScreen.cpp
	quotesParser.cpp
	trader.cpp
	levels.cpp
	entryAnalyzer.cpp
	EMACross.cpp
)

target_link_libraries(robotradeutils
	chart
	tz
	fmt
	${JSONCPP_LIBRARIES}
	boost_program_options
	Threads::Threads
)

target_link_libraries(robotrade robotradeutils)
target_link_libraries(quotetool robotradeutils)
