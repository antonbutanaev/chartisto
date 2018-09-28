find_package(Boost REQUIRED)
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
    Levels.cpp
    EntryAnalyzer.cpp
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
set(LEVELS_JSON levels.json)

configure_file(${LOG_CONF} ${LOG_CONF} COPYONLY)
configure_file(${LEVELS_JSON} ${LEVELS_JSON} COPYONLY)
