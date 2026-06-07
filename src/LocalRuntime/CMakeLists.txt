add_library(LocalRuntime STATIC LocalRuntimeEngine.cpp LlamaRuntime.cpp)
target_include_directories(LocalRuntime PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(LocalRuntime PUBLIC llama)
