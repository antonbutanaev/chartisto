add_library(chart
	chart.cpp
	indicators.cpp
	reduce.cpp
	stepFind.cpp
)

target_link_libraries(chart Threads::Threads)
