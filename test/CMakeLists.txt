enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(testchart
	testIndicators.cpp
	testChart.cpp
	testConstants.cpp
)

target_link_libraries(testchart
	robotradeutils
	${GTEST_BOTH_LIBRARIES}
)

add_executable(testrobotrade
	testRobotrade.cpp
)

target_link_libraries(testrobotrade
	robotradeutils
	${GTEST_BOTH_LIBRARIES}
)

add_executable(testutil testUtil.cpp)

target_link_libraries(testutil
	${GTEST_BOTH_LIBRARIES}
)
