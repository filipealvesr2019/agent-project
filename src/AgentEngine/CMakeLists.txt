add_library(AgentEngine STATIC Agent.cpp AgentEngine.cpp)
target_link_libraries(AgentEngine PRIVATE ReasoningTimelineEngine)
