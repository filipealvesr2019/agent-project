add_library(VisionEngine STATIC VisionEngine.cpp)
target_include_directories(VisionEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(VisionEngine PRIVATE cxx_std_20)