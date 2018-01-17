file(GLOB SRC *.cpp)

add_executable(testpainter ${SRC})

target_link_libraries(testpainter
	painter
	gtest
	gtest_main
)
