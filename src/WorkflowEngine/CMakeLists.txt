add_library(WorkflowEngine STATIC WorkflowEngine.cpp)
target_include_directories(WorkflowEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
