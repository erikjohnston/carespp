cmake_minimum_required(VERSION 3.1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors")

add_library(caresppObjects OBJECT src/arespp.cpp src/buffer_view.cpp)

target_include_directories(caresppObjects PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_options(caresppObjects PUBLIC "-std=c++11" "-fPIC")

add_library(carespp SHARED $<TARGET_OBJECTS:caresppObjects>)
target_link_libraries(carespp cares)

add_library(caresppStatic STATIC $<TARGET_OBJECTS:caresppObjects>)
target_link_libraries(caresppStatic cares)

install(TARGETS carespp caresppStatic EXPORT caresppConfig
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/carespp DESTINATION include)

install(EXPORT caresppConfig DESTINATION lib/carespp)
