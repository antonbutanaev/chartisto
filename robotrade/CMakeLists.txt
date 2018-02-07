find_package(Boost)
find_package(jsoncpp)

add_executable(robotrade
    quotesparser.cpp
    main.cpp
)

target_link_libraries(robotrade
	tz
	jsoncpp_lib
	boost_program_options
	chart
)
