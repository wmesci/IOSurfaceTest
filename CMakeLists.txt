cmake_minimum_required(VERSION 3.12)
project(IOSurfaceTest)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DGL_SILENCE_DEPRECATION)

find_package(OpenGL REQUIRED)

set(LIBS OpenGL::GL "-framework Cocoa" "-framework IOSurface" "-framework CoreVideo" "-framework CoreFoundation")

add_executable(server server.cpp)
target_link_libraries(server PRIVATE ${LIBS})

add_executable(client client.mm)
target_link_libraries(client PRIVATE ${LIBS})
