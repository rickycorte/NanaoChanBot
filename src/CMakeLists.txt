set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/lib/logging)
include_directories(global)


add_executable(HikariBackend http/Reply.cpp
        http/Reply.hpp
        http/Request.cpp
        http/Request.hpp
        CMakeLists.txt
        Server.cpp
        Server.hpp
        Source.cpp)
