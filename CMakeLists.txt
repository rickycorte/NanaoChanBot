cmake_minimum_required(VERSION 3.12)
project(NanaoChanBot)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(lib)
include_directories(lib/logging)
include_directories(lib/logging/rickycorte)
include_directories(lib/mitie/dlib)
include_directories(lib/mitie/mitielib/include)
include_directories(lib/json/include)
include_directories(src)
include_directories(src/global)
include_directories(src/http)


add_custom_command(
OUTPUT ${CMAKE_SOURCE_DIR}/lib/mitie/mitielib/libmitie.a
COMMAND make
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/mitie
 )

add_custom_target(
mitielib
DEPENDS ${CMAKE_SOURCE_DIR}/lib/mitie/mitielib/libmitie.a
)

add_executable(NanaoChanBot
        lib/logging/rickycorte/Logging.hpp
        src/global/GlobalStaticConfig.hpp
        src/http/ApiInterface.hpp
        src/http/Reply.cpp
        src/http/Reply.hpp
        src/http/Request.cpp
        src/http/Request.hpp
        src/CMakeLists.txt
        src/EchoApi.hpp
        src/Server.cpp
        src/Server.hpp
        src/Source.cpp
        CMakeLists.txt
        LICENSE
        README.md src/traier/MLTrainer.cpp src/traier/MLTrainer.hpp src/global/Utility.h)

target_link_libraries(NanaoChanBot pthread ${CMAKE_SOURCE_DIR}/lib/mitie/mitielib/libmitie.a)
add_dependencies(NanaoChanBot mitielib)