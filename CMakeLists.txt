cmake_minimum_required(VERSION 3.12)
project(lsbi)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads)

set(CMAKE_CXX_FLAGS_RELEASE  "-O3")
#set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")


file(GLOB SOURCE_FILES "src/*.cpp")

add_library(learnedindex STATIC ${SOURCE_FILES})
target_include_directories(learnedindex PUBLIC ${CMAKE_HOME_DIRECTORY}/headers)


if(WIN32)

	add_executable(bucket app/bucket.cpp)
	target_link_libraries(bucket PUBLIC learnedindex ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_HOME_DIRECTORY}/lib/spatialindex_c-64.lib ${CMAKE_HOME_DIRECTORY}/lib/spatialindex-64.lib)

	add_executable(rtree app/rtree.cpp)
	target_link_libraries(rtree PUBLIC learnedindex ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_HOME_DIRECTORY}/lib/spatialindex_c-64.lib ${CMAKE_HOME_DIRECTORY}/lib/spatialindex-64.lib)

endif(WIN32)

if(UNIX)

	add_executable(bucket app/bucket.cpp)
	target_link_libraries(bucket PUBLIC learnedindex ${CMAKE_THREAD_LIBS_INIT} spatialindex_c spatialindex)

	add_executable(rtree app/rtree.cpp)
	target_link_libraries(rtree PUBLIC learnedindex ${CMAKE_THREAD_LIBS_INIT} spatialindex_c spatialindex)

endif(UNIX)