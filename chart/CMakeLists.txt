add_library(chart
	chart.cpp
	indicators.cpp
	reduce.cpp
)

target_link_libraries(chart Threads::Threads)
