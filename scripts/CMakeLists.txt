cmake_minimum_required(VERSION 2.8)

set(PREFIX ../)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PREFIX})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PREFIX})
file(GLOB_RECURSE SRC ${PREFIX}/src/*.cpp)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)
pkg_check_modules(GTHREAD REQUIRED gthread-2.0)
pkg_check_modules(PCRECPP REQUIRED libpcrecpp)

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${PREFIX}/include ${GTKMM_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS} ${PCRECPP_INCLUDE_DIRS})
add_executable(main ${SRC})
target_link_libraries(main ${GTKMM_LIBRARIES} ${GTHREAD_LIBRARIES} ${OPENSSL_LIBRARIES} ${PCRECPP_LIBRARIES})