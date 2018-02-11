enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(testchart
	testindicators.cpp
	testchart.cpp
	testconstants.cpp
)

target_link_libraries(testchart
	chart
	robotradeutils
	${GTEST_BOTH_LIBRARIES}
)

add_executable(testrobotrade
	testrobotrade.cpp
)

target_link_libraries(testrobotrade
	robotradeutils
	${GTEST_BOTH_LIBRARIES}
)
