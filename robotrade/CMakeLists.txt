find_package(Boost)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

add_executable(robotrade
    main.cpp
)

add_library(robotradeutils
	triplescreen.cpp
    quotesparser.cpp
)

target_link_libraries(robotrade
	tz
	boost_program_options
	chart
	robotradeutils
	${JSONCPP_LIBRARIES}
)
