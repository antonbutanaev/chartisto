file(GLOB SRC *.cpp)

add_executable(testchart ${SRC})

target_link_libraries(testchart
	chart
	gtest
	gtest_main
)
