cmake_minimum_required(VERSION 3.20)
project(AgentOS VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:char8_t-)
endif()
# Includes
include_directories(include)

# Configuração do JUCE
# include(FetchContent)
# FetchContent_Declare(
#     juce
#     URL "https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.0.zip"
# )
# FetchContent_MakeAvailable(juce)

# Define as subpastas (módulos)
# add_subdirectory(src/UI)
add_subdirectory(src/AgentEngine)
add_subdirectory(src/WorkflowEngine)
add_subdirectory(src/MemoryEngine)
add_subdirectory(src/ToolEngine)
add_subdirectory(src/PlannerEngine)
add_subdirectory(src/WorkspaceEngine)
add_subdirectory(src/SemanticSearchEngine)
add_subdirectory(src/KnowledgeGraphEngine)
add_subdirectory(src/ASTEngine)
add_subdirectory(src/VerificationEngine)
add_subdirectory(src/EventBus)
add_subdirectory(src/ModelRouter)
add_subdirectory(src/GitEngine)
add_subdirectory(src/ResearchEngine)
add_subdirectory(src/MonitoringEngine)
add_subdirectory(src/PersistenceEngine)
add_subdirectory(src/GovernanceEngine)
add_subdirectory(src/Sandbox)
add_subdirectory(src/ChangeManagement)
add_subdirectory(src/AgentProfiles)
add_subdirectory(src/ReasoningEngine)
add_subdirectory(src/ContextEngine)
add_subdirectory(src/CapabilityEngine)
add_subdirectory(src/MemoryAPI)
add_subdirectory(src/MultiAgentCollaboration)
add_subdirectory(src/PluginManager)
add_subdirectory(src/PromptEngine)
add_subdirectory(src/ObjectiveEngine)
add_subdirectory(src/CostMonitor)
add_subdirectory(src/ModelManager)
add_subdirectory(src/WindowsBuildManager)
add_subdirectory(src/VisionEngine)
add_subdirectory(src/LocalRuntime)
add_subdirectory(src/ProjectManager)
add_subdirectory(src/OrganizationEngine)
add_subdirectory(src/ReasoningTimelineEngine)
add_subdirectory(src/Cognitive)

# Executável Principal JUCE
# juce_add_gui_app(AgentOS
#     PRODUCT_NAME "AgentOS"
#     VERSION 1.0.0
# )
# 
# target_sources(AgentOS PRIVATE src/main.cpp src/ChangeManagement/ChangeManagement.cpp)

# target_link_libraries(AgentOS PRIVATE 
#     UI
#     AgentEngine 
#     WorkflowEngine
#     EventBus 
#     MemoryEngine 
#     GovernanceEngine
#     Sandbox
#     ChangeManagement
#     AgentProfiles
#     PlannerEngine
#     ModelRouter
#     ReasoningEngine
#     ContextEngine
#     CapabilityEngine
#     MemoryAPI
#     MonitoringEngine
#     KnowledgeGraphEngine
#     MultiAgentCollaboration
#     PluginManager
#     PromptEngine
#     ObjectiveEngine
#     CostMonitor
#     ModelManager
#     WindowsBuildManager
#     VisionEngine
#     LocalRuntime
#     ProjectManager
#     OrganizationEngine
#     ReasoningTimelineEngine
#     ToolEngine
#     sqlite
# )

add_library(sqlite STATIC libs/sqlite/sqlite3.c)
target_include_directories(sqlite PUBLIC libs/sqlite)
# target_link_libraries(AgentOS PRIVATE MemoryEngine sqlite)

# Integration test suite
option(BUILD_INTEGRATION_TESTS "Build AgentOS integration test suite" OFF)
if(BUILD_INTEGRATION_TESTS)
    add_executable(AgentOS_Integration
        tests/run_agentos_tests.cpp
        src/AgentEngine/Agent.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/ToolEngine/ToolEngine.cpp
        src/ChangeManagement/ChangeManagement.cpp
        src/GovernanceEngine/GovernanceEngine.cpp
        src/Sandbox/Sandbox.cpp
        src/WorkflowEngine/WorkflowEngine.cpp
        src/PlannerEngine/PlannerEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/AgentProfiles/AgentProfiles.cpp
        src/ReasoningEngine/ReasoningEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/CapabilityEngine/CapabilityEngine.cpp
        src/PromptEngine/PromptEngine.cpp
        src/ObjectiveEngine/ObjectiveEngine.cpp
        src/CostMonitor/CostMonitor.cpp
        src/ModelManager/ModelManager.cpp
        src/WindowsBuildManager/WindowsBuildManager.cpp
    )
    target_include_directories(AgentOS_Integration PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_Integration PRIVATE sqlite)
    target_compile_features(AgentOS_Integration PRIVATE cxx_std_20)
endif()

# Workflow test suite
option(BUILD_WORKFLOW_TESTS "Build AgentOS workflow test" OFF)
if(BUILD_WORKFLOW_TESTS)
    add_executable(AgentOS_WorkflowTest
        tests/test_workflow_full.cpp
        src/AgentEngine/Agent.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/ToolEngine/ToolEngine.cpp
        src/ChangeManagement/ChangeManagement.cpp
        src/GovernanceEngine/GovernanceEngine.cpp
        src/Sandbox/Sandbox.cpp
        src/WorkflowEngine/WorkflowEngine.cpp
        src/PlannerEngine/PlannerEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/AgentProfiles/AgentProfiles.cpp
        src/ReasoningEngine/ReasoningEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/CapabilityEngine/CapabilityEngine.cpp
        src/PromptEngine/PromptEngine.cpp
        src/ObjectiveEngine/ObjectiveEngine.cpp
        src/CostMonitor/CostMonitor.cpp
        src/ModelManager/ModelManager.cpp
        src/WindowsBuildManager/WindowsBuildManager.cpp
    )
    target_include_directories(AgentOS_WorkflowTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_WorkflowTest PRIVATE sqlite)
    target_compile_features(AgentOS_WorkflowTest PRIVATE cxx_std_20)
endif()

# Phase 6 test suite
option(BUILD_PHASE6_TESTS "Build AgentOS Phase 6 tests" OFF)
if(BUILD_PHASE6_TESTS)
    add_executable(AgentOS_Phase6
        tests/test_phase6.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/ToolEngine/ToolEngine.cpp
        src/ChangeManagement/ChangeManagement.cpp
        src/GovernanceEngine/GovernanceEngine.cpp
        src/Sandbox/Sandbox.cpp
        src/WorkflowEngine/WorkflowEngine.cpp
        src/PlannerEngine/PlannerEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/AgentProfiles/AgentProfiles.cpp
        src/ReasoningEngine/ReasoningEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/CapabilityEngine/CapabilityEngine.cpp
        src/PromptEngine/PromptEngine.cpp
        src/ObjectiveEngine/ObjectiveEngine.cpp
        src/CostMonitor/CostMonitor.cpp
        src/ModelManager/ModelManager.cpp
        src/WindowsBuildManager/WindowsBuildManager.cpp
    )
    target_include_directories(AgentOS_Phase6 PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_Phase6 PRIVATE sqlite)
    target_compile_features(AgentOS_Phase6 PRIVATE cxx_std_20)
endif()

# ModelManager test suite
option(BUILD_MODELMANAGER_TESTS "Build AgentOS ModelManager tests" OFF)
if(BUILD_MODELMANAGER_TESTS)
    add_executable(AgentOS_ModelManagerTest
        tests/test_modelmanager.cpp
        src/ModelManager/ModelManager.cpp
        src/WindowsBuildManager/WindowsBuildManager.cpp
    )
    target_include_directories(AgentOS_ModelManagerTest PRIVATE include)
    target_compile_features(AgentOS_ModelManagerTest PRIVATE cxx_std_20)
    target_compile_options(AgentOS_ModelManagerTest PRIVATE /EHsc)
endif()

# WindowsBuildManager test suite
option(BUILD_WINDOWSBUILD_TESTS "Build AgentOS WindowsBuildManager tests" OFF)
if(BUILD_WINDOWSBUILD_TESTS)
    add_executable(AgentOS_WindowsBuildTest
        tests/test_windowsbuild.cpp
        src/WindowsBuildManager/WindowsBuildManager.cpp
    )
    target_include_directories(AgentOS_WindowsBuildTest PRIVATE include)
    target_compile_features(AgentOS_WindowsBuildTest PRIVATE cxx_std_20)
endif()

# VisionEngine test suite
option(BUILD_VISION_TESTS "Build AgentOS VisionEngine tests" OFF)
if(BUILD_VISION_TESTS)
    add_executable(AgentOS_VisionTest
        tests/test_visionengine.cpp
        src/VisionEngine/VisionEngine.cpp
    )
    target_include_directories(AgentOS_VisionTest PRIVATE include)
    target_compile_features(AgentOS_VisionTest PRIVATE cxx_std_20)
endif()

# ContextEngine test suite
option(BUILD_CONTEXT_TESTS "Build AgentOS ContextEngine tests" OFF)
if(BUILD_CONTEXT_TESTS)
    add_executable(AgentOS_ContextTest
        tests/test_contextengine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/MemoryEngine/MemoryEngine.cpp
    )
    target_include_directories(AgentOS_ContextTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_ContextTest PRIVATE sqlite)
    target_compile_features(AgentOS_ContextTest PRIVATE cxx_std_20)
endif()

# CapabilityEngine test suite
option(BUILD_CAPABILITY_TESTS "Build AgentOS CapabilityEngine tests" OFF)
if(BUILD_CAPABILITY_TESTS)
    add_executable(AgentOS_CapabilityTest
        tests/test_capabilityengine.cpp
        src/CapabilityEngine/CapabilityEngine.cpp
    )
    target_include_directories(AgentOS_CapabilityTest PRIVATE include)
    target_compile_features(AgentOS_CapabilityTest PRIVATE cxx_std_20)
endif()

# LocalRuntime test suite
option(BUILD_LOCALRUNTIME_TESTS "Build AgentOS LocalRuntime tests" OFF)
if(BUILD_LOCALRUNTIME_TESTS)
    add_executable(AgentOS_LocalRuntimeTest
        tests/test_localruntime.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
    )
    target_include_directories(AgentOS_LocalRuntimeTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_LocalRuntimeTest PRIVATE sqlite)
    target_compile_features(AgentOS_LocalRuntimeTest PRIVATE cxx_std_20)
endif()

# Phase 11 TEE test suite
option(BUILD_PHASE11_TESTS "Build AgentOS Phase 11 TEE tests" OFF)
if(BUILD_PHASE11_TESTS)
    add_executable(AgentOS_Phase11Test
        tests/test_phase11_tee.cpp
        src/ToolEngine/ToolEngine.cpp
        src/ToolEngine/Phase11Tools.cpp
    )
    target_include_directories(AgentOS_Phase11Test PRIVATE include)
    target_compile_features(AgentOS_Phase11Test PRIVATE cxx_std_20)
endif()

# Phase 12 Memory & Knowledge test suite
option(BUILD_PHASE12_TESTS "Build AgentOS Phase 12 tests" OFF)
if(BUILD_PHASE12_TESTS)
    add_executable(AgentOS_MemoryV2Test
        tests/test_memoryengine_v2.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/MemoryAPI/MemoryAPI.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
    )
    target_include_directories(AgentOS_MemoryV2Test PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_MemoryV2Test PRIVATE sqlite)
    target_compile_features(AgentOS_MemoryV2Test PRIVATE cxx_std_20)
endif()

# Phase 13 Multi-Agent Collaboration test suite
option(BUILD_PHASE13_TESTS "Build AgentOS Phase 13 tests" OFF)
if(BUILD_PHASE13_TESTS)
    add_executable(AgentOS_MultiAgentTest
        tests/test_multiagent.cpp
        src/MultiAgentCollaboration/MultiAgentCollaboration.cpp
    )
    target_include_directories(AgentOS_MultiAgentTest PRIVATE include)
    target_compile_features(AgentOS_MultiAgentTest PRIVATE cxx_std_20)
endif()

# Phase 15 Plugin Marketplace test suite
option(BUILD_PHASE15_TESTS "Build AgentOS Phase 15 tests" OFF)
if(BUILD_PHASE15_TESTS)
    add_executable(AgentOS_PluginMarketplaceTest
        tests/test_pluginmarketplace.cpp
        src/PluginManager/PluginManager.cpp
        src/ToolEngine/Phase11Tools.cpp
    )
    target_include_directories(AgentOS_PluginMarketplaceTest PRIVATE include)
    target_compile_features(AgentOS_PluginMarketplaceTest PRIVATE cxx_std_20)
endif()

# Hardening & Validation test suite
option(BUILD_HARDENING_TESTS "Build AgentOS Hardening tests" OFF)
if(BUILD_HARDENING_TESTS)
    add_executable(AgentOS_HardeningTest
        tests/test_hardening_validation.cpp
        src/MultiAgentCollaboration/MultiAgentCollaboration.cpp
        src/PluginManager/PluginManager.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/MemoryAPI/MemoryAPI.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
        src/ToolEngine/Phase11Tools.cpp
    )
    target_include_directories(AgentOS_HardeningTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_HardeningTest PRIVATE sqlite)
    target_compile_features(AgentOS_HardeningTest PRIVATE cxx_std_20)
endif()

# Performance & Stress test suite
option(BUILD_PERFORMANCE_TESTS "Build AgentOS Performance tests" OFF)
if(BUILD_PERFORMANCE_TESTS)
    add_executable(AgentOS_PerformanceTest
        tests/test_performance_stress.cpp
        src/MonitoringEngine/MonitoringEngine.cpp
        src/PluginManager/PluginManager.cpp
        src/ToolEngine/Phase11Tools.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
        src/MemoryAPI/MemoryAPI.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
        src/MemoryEngine/MemoryEngine.cpp
    )
    target_include_directories(AgentOS_PerformanceTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_PerformanceTest PRIVATE sqlite)
    target_compile_features(AgentOS_PerformanceTest PRIVATE cxx_std_20)
endif()

# Test runner (opcional, compilar com -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build AgentOS test runner" OFF)
if(BUILD_TESTS)
    add_executable(AgentOS_Tests
        src/test_runner.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/ToolEngine/ToolEngine.cpp
        src/ChangeManagement/ChangeManagement.cpp
    )
    target_include_directories(AgentOS_Tests PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_Tests PRIVATE sqlite)
    target_compile_features(AgentOS_Tests PRIVATE cxx_std_20)
endif()

# Organization Workflow Test Suite
option(BUILD_ORG_WORKFLOW_TESTS "Build AgentOS Organization Workflow tests" ON)
if(BUILD_ORG_WORKFLOW_TESTS)
    add_executable(AgentOS_OrgWorkflowTest
        tests/test_workflow_organization.cpp
        src/WorkflowEngine/WorkflowEngine.cpp
        src/OrganizationEngine/OrganizationEngine.cpp
        src/ProjectManager/ProjectManager.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
    )
    target_include_directories(AgentOS_OrgWorkflowTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_OrgWorkflowTest PRIVATE sqlite)
    target_compile_features(AgentOS_OrgWorkflowTest PRIVATE cxx_std_20)
endif()

# Phase 16.5 Hell Test Framework
option(BUILD_HELL_TESTS "Build AgentOS Hell Scalability Tests" ON)
if(BUILD_HELL_TESTS)
    add_executable(AgentOS_HellTest
        tests/test_hell_scale.cpp
        src/OrganizationEngine/OrganizationEngine.cpp
        src/ProjectManager/ProjectManager.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
    )
    target_include_directories(AgentOS_HellTest PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_HellTest PRIVATE sqlite)
    target_compile_features(AgentOS_HellTest PRIVATE cxx_std_20)
endif()

# Phase 16.8 Scalability & Benchmark Framework
option(BUILD_BENCHMARK_RUNNER "Build AgentOS Benchmark Runner" ON)
if(BUILD_BENCHMARK_RUNNER)
    add_executable(AgentOS_BenchmarkRunner
        tests/benchmark_runner.cpp
        src/OrganizationEngine/OrganizationEngine.cpp
        src/ProjectManager/ProjectManager.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
        src/LocalRuntime/TaskScheduler.cpp
        src/LocalRuntime/ModelPoolManager.cpp
    )
    target_include_directories(AgentOS_BenchmarkRunner PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_BenchmarkRunner PRIVATE sqlite)
    target_compile_features(AgentOS_BenchmarkRunner PRIVATE cxx_std_20)
endif()

# Phase 16.11 Memory Leak Apocalypse
option(BUILD_MEMORY_LEAK_APOCALYPSE "Build AgentOS Memory Leak Test" ON)
if(BUILD_MEMORY_LEAK_APOCALYPSE)
    add_executable(AgentOS_MemoryLeakApocalypse
        tests/memory_leak_apocalypse.cpp
        src/OrganizationEngine/OrganizationEngine.cpp
        src/ProjectManager/ProjectManager.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
        src/LocalRuntime/TaskScheduler.cpp
        src/LocalRuntime/ModelPoolManager.cpp
    )
    target_link_libraries(AgentOS_MemoryLeakApocalypse PRIVATE psapi)
    target_include_directories(AgentOS_MemoryLeakApocalypse PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_MemoryLeakApocalypse PRIVATE sqlite)
    target_compile_features(AgentOS_MemoryLeakApocalypse PRIVATE cxx_std_20)
endif()

# Phase 16.12 Surrogate Decision Layer
option(BUILD_SURROGATE_TEST "Build AgentOS Surrogate Router Test" ON)
if(BUILD_SURROGATE_TEST)
    add_executable(AgentOS_SurrogateTest
        tests/test_surrogate.cpp
        src/SurrogateDecisionLayer/SurrogateRouter.cpp
    )
    target_include_directories(AgentOS_SurrogateTest PRIVATE include)
    target_compile_features(AgentOS_SurrogateTest PRIVATE cxx_std_20)
endif()

# Phase 16.13 Experience Replay Engine
option(BUILD_EXPERIENCE_REPLAY_TEST "Build AgentOS Experience Replay Test" ON)
if(BUILD_EXPERIENCE_REPLAY_TEST)
    add_executable(AgentOS_ExperienceReplayTest
        tests/test_experience_replay.cpp
        src/SurrogateDecisionLayer/SurrogateRouter.cpp
        src/ExperienceReplayEngine/ExperienceReplayEngine.cpp
    )
    target_include_directories(AgentOS_ExperienceReplayTest PRIVATE include)
    target_compile_features(AgentOS_ExperienceReplayTest PRIVATE cxx_std_20)
endif()

# Phase 16.14 Runtime Learning Layer
option(BUILD_RUNTIME_LEARNING_TEST "Build AgentOS Runtime Learning Test" ON)
if(BUILD_RUNTIME_LEARNING_TEST)
    add_executable(AgentOS_RuntimeLearningTest
        tests/test_runtime_learning.cpp
        src/SurrogateDecisionLayer/SurrogateRouter.cpp
        src/RuntimeLearningLayer/RuntimeLearningLayer.cpp
    )
    target_include_directories(AgentOS_RuntimeLearningTest PRIVATE include)
    target_compile_features(AgentOS_RuntimeLearningTest PRIVATE cxx_std_20)
endif()

# Phase 16.15 & 16.16 Hardware Adaptive & HellBreaker 3.0
option(BUILD_HELL_BREAKER_3 "Build AgentOS Hell Breaker 3.0 Test" ON)
if(BUILD_HELL_BREAKER_3)
    add_executable(AgentOS_HellBreaker3
        tests/test_hell_breaker_3.cpp
        src/OrganizationEngine/OrganizationEngine.cpp
        src/ProjectManager/ProjectManager.cpp
        src/AgentEngine/Agent.cpp
        src/ReasoningTimelineEngine/ReasoningTimelineEngine.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/KnowledgeGraphEngine/KnowledgeGraphEngine.cpp
        src/ContextEngine/ContextEngine.cpp
        src/ModelRouter/ModelRouter.cpp
        src/LocalRuntime/LocalRuntimeEngine.cpp
        src/LocalRuntime/TaskScheduler.cpp
        src/LocalRuntime/ModelPoolManager.cpp
        src/SurrogateDecisionLayer/SurrogateRouter.cpp
        src/ExperienceReplayEngine/ExperienceReplayEngine.cpp
        src/RuntimeLearningLayer/RuntimeLearningLayer.cpp
        src/HardwareAdaptiveRuntime/HardwareProfileManager.cpp
        src/LocalRuntime/LlamaRuntime.cpp
    )
    target_include_directories(AgentOS_HellBreaker3 PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_HellBreaker3 PRIVATE sqlite llama)
    target_compile_features(AgentOS_HellBreaker3 PRIVATE cxx_std_20)
endif()

# Phase 16.17 Real Inference Runtime
option(BUILD_REAL_INFERENCE_TEST "Build AgentOS Real Inference Test" ON)
if(BUILD_REAL_INFERENCE_TEST)
    add_executable(AgentOS_RealInferenceTest
        tests/test_real_inference.cpp
        src/LocalRuntime/LlamaRuntime.cpp
    )
    target_include_directories(AgentOS_RealInferenceTest PRIVATE include)
    target_link_libraries(AgentOS_RealInferenceTest PRIVATE llama)
    target_compile_features(AgentOS_RealInferenceTest PRIVATE cxx_std_20)
endif()

# --- llama.cpp ---
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "Disable llama tests")
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "Disable llama examples")
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "Disable llama server")
add_subdirectory(libs/llama.cpp)

# Phase 1: Cognitive Engine / Model Registry Test
option(BUILD_COGNITIVE_TESTS "Build AgentOS Cognitive Tests" ON)
if(BUILD_COGNITIVE_TESTS)
    add_executable(AgentOS_ModelRegistryTest
        tests/test_model_registry.cpp
    )
    target_include_directories(AgentOS_ModelRegistryTest PRIVATE include)
    target_link_libraries(AgentOS_ModelRegistryTest PRIVATE Cognitive)
    target_compile_features(AgentOS_ModelRegistryTest PRIVATE cxx_std_20)

    # Semana 2: Full Cognitive Pipeline (TaskAnalyzer + SurrogateRouter)
    add_executable(AgentOS_CognitivePipelineTest
        tests/test_cognitive_pipeline.cpp
    )
    target_include_directories(AgentOS_CognitivePipelineTest PRIVATE include)
    target_link_libraries(AgentOS_CognitivePipelineTest PRIVATE Cognitive)
    target_compile_features(AgentOS_CognitivePipelineTest PRIVATE cxx_std_20)

    # Semana 3: Observability - MetricsCollector + BottleneckDetector + AutoRecovery
    add_executable(AgentOS_ObservabilityTest
        tests/test_observability.cpp
    )
    target_include_directories(AgentOS_ObservabilityTest PRIVATE include)
    target_link_libraries(AgentOS_ObservabilityTest PRIVATE Cognitive)
    target_compile_features(AgentOS_ObservabilityTest PRIVATE cxx_std_20)

    # Semana 4: Learning Engine - Score adaptativo + historico de decisoes
    add_executable(AgentOS_LearningEngineTest
        tests/test_learning_engine.cpp
    )
    target_include_directories(AgentOS_LearningEngineTest PRIVATE include)
    target_link_libraries(AgentOS_LearningEngineTest PRIVATE Cognitive)
    target_compile_features(AgentOS_LearningEngineTest PRIVATE cxx_std_20)

    # Semana 5: Context Management Engine - Token budget + trimming
    add_executable(AgentOS_ContextManagerTest
        tests/test_context_manager.cpp
    )
    target_include_directories(AgentOS_ContextManagerTest PRIVATE include)
    target_link_libraries(AgentOS_ContextManagerTest PRIVATE Cognitive)
    target_compile_features(AgentOS_ContextManagerTest PRIVATE cxx_std_20)

    # Semana 6: Episodic Memory Engine - Persistencia jsonl
    add_executable(AgentOS_MemoryEngineTest
        tests/test_memory_engine.cpp
    )
    target_include_directories(AgentOS_MemoryEngineTest PRIVATE include)
    target_link_libraries(AgentOS_MemoryEngineTest PRIVATE Cognitive)
    target_compile_features(AgentOS_MemoryEngineTest PRIVATE cxx_std_20)

    # Semana 7: Knowledge Base - Consolidacao de memorias em documentos Markdown/JSON
    add_executable(AgentOS_KnowledgeBaseTest
        tests/test_knowledge_base.cpp
    )
    target_include_directories(AgentOS_KnowledgeBaseTest PRIVATE include)
    target_link_libraries(AgentOS_KnowledgeBaseTest PRIVATE Cognitive)
    target_compile_features(AgentOS_KnowledgeBaseTest PRIVATE cxx_std_20)

    # Semana 8: Vector Search - Similaridade de Cossenos e RAG em Memoria Episodica
    add_executable(AgentOS_VectorSearchTest
        tests/test_vector_search.cpp
    )
    target_include_directories(AgentOS_VectorSearchTest PRIVATE include)
    target_link_libraries(AgentOS_VectorSearchTest PRIVATE Cognitive)
    target_compile_features(AgentOS_VectorSearchTest PRIVATE cxx_std_20)

    # Semana 9: Multi-Agent Orchestrator - Agentes especializados
    add_executable(AgentOS_MultiAgentTest
        tests/test_multi_agent.cpp
    )
    target_include_directories(AgentOS_MultiAgentTest PRIVATE include)
    target_link_libraries(AgentOS_MultiAgentTest PRIVATE Cognitive)
    target_compile_features(AgentOS_MultiAgentTest PRIVATE cxx_std_20)
endif()



