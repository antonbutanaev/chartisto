find_package(Boost)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(LOG4CPLUS REQUIRED log4cplus)

add_executable(robotrade
    robotrade.cpp
)

add_library(robotradeutils
	triplescreen.cpp
    quotesparser.cpp
    trader.cpp
)

target_link_libraries(robotradeutils
	chart
	tz
	fmt
	${JSONCPP_LIBRARIES}
	${LOG4CPLUS_LIBRARIES}
)

target_link_libraries(robotrade
	boost_program_options
	robotradeutils
)

set(LOG_CONF robotrade_log.conf)

configure_file(${LOG_CONF} ${LOG_CONF} COPYONLY)
