add_library(MemoryEngine STATIC MemoryEngine.cpp)
target_include_directories(MemoryEngine PRIVATE "${CMAKE_SOURCE_DIR}/libs/sqlite")
target_link_libraries(MemoryEngine PRIVATE sqlite)
