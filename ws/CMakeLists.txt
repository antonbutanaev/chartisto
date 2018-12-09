add_executable(pipeedit pipeEdit.cpp)
add_executable(pipesave pipeSave.cpp)
add_executable(decodews decodeWs.cpp)

target_link_libraries(pipeedit Threads::Threads)
