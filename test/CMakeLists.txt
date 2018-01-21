enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(testchart
	testchart.cpp
	testconstants.cpp
)

target_link_libraries(testchart
	chart
	${GTEST_BOTH_LIBRARIES}
)
