add_library(Cognitive STATIC
    ModelRegistry.cpp
    TaskAnalyzer.cpp
    SurrogateRouter.cpp
    MetricsCollector.cpp
    BottleneckDetector.cpp
    AutoRecovery.cpp
    LearningEngine.cpp
    ContextManager.cpp
    MemoryEngine.cpp
    KnowledgeBase.cpp
    VectorSearch.cpp
    Orchestrator.cpp
)

target_include_directories(Cognitive PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(Cognitive PUBLIC cxx_std_20)
target_link_libraries(Cognitive PUBLIC psapi)
