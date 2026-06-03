add_library(Cognitive STATIC
    ModelRegistry.cpp
)

target_include_directories(Cognitive PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(Cognitive PUBLIC cxx_std_20)
