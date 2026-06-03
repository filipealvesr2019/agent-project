add_library(GovernanceEngine STATIC GovernanceEngine.cpp)
target_include_directories(GovernanceEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(GovernanceEngine PUBLIC EventBus)
