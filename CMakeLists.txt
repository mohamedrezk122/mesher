cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mesher)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
find_package(SDL2 REQUIRED)
find_package(assimp REQUIRED)

add_executable(mesher ${SRC_FILES})

target_include_directories(mesher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(mesher PUBLIC ${ASSIMP_INCLUDE_DIRS})
target_include_directories(mesher PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(mesher ${SDL2_LIBRARIES})
target_link_libraries(mesher ${ASSIMP_LIBRARIES})

