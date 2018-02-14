find_package(Boost)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
pkg_check_modules(LOG4CPLUS log4cplus)

add_executable(robotrade
    main.cpp
)

add_library(robotradeutils
	triplescreen.cpp
    quotesparser.cpp
)

target_link_libraries(robotradeutils
	${JSONCPP_LIBRARIES}
	${LOG4CPLUS_LIBRARIES}
)

target_link_libraries(robotrade
	tz
	fmt
	boost_program_options
	chart
	robotradeutils
)

configure_file(robotrade.ini robotrade.ini COPYONLY)
