cmake_minimum_required(VERSION 3.19)
project(screener)

find_package(Boost 1.75 COMPONENTS program_options system filesystem REQUIRED)
find_package(OpenSSL)
add_definitions(-DBOOST_BEAST_USE_OPENSSL=1)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -fpermissive -Wno-missing-field-initializers -g -ggdb"
)

add_executable(screener
	Screener.cpp
	TiingoApi.cpp TiingoApi.h
	SyncQuotes.cpp SyncQuotes.h
	Screen.cpp Screen.h
	ShowRet13612.cpp ShowRet13612.h
	ParseQuotess.cpp ParseQuotess.h
	Calcs.cpp Calcs.h
)

target_link_libraries(screener
	PRIVATE
	Boost::program_options
	Boost::filesystem
	Boost::system
	Threads::Threads
	OpenSSL::SSL
	OpenSSL::Crypto
	${JSONCPP_LIBRARIES}
	date::date
)

set_target_properties(screener PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
)