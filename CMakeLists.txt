cmake_minimum_required(VERSION 3.20)
project(AgentOS VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(include)

# Configuração do JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    URL "https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.0.zip"
)
FetchContent_MakeAvailable(juce)

# Define as subpastas (módulos)
add_subdirectory(src/UI)
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
add_subdirectory(src/PromptEngine)
add_subdirectory(src/ObjectiveEngine)
add_subdirectory(src/CostMonitor)
add_subdirectory(src/ModelManager)
add_subdirectory(src/WindowsBuildManager)

# Executável Principal JUCE
juce_add_gui_app(AgentOS
    PRODUCT_NAME "AgentOS"
    VERSION 1.0.0
)

    target_sources(AgentOS PRIVATE src/main.cpp src/ChangeManagement/ChangeManagement.cpp)

target_link_libraries(AgentOS PRIVATE 
    UI
    AgentEngine 
    WorkflowEngine
    EventBus 
    MemoryEngine 
    GovernanceEngine
    Sandbox
    ChangeManagement
    AgentProfiles
    PlannerEngine
    ModelRouter
    ReasoningEngine
    ContextEngine
    PromptEngine
    ObjectiveEngine
    CostMonitor
    ModelManager
    WindowsBuildManager
    sqlite
)

add_library(sqlite STATIC libs/sqlite/sqlite3.c)
target_include_directories(sqlite PUBLIC libs/sqlite)
target_link_libraries(AgentOS PRIVATE MemoryEngine sqlite)

# Integration test suite
option(BUILD_INTEGRATION_TESTS "Build AgentOS integration test suite" OFF)
if(BUILD_INTEGRATION_TESTS)
    add_executable(AgentOS_Integration
        tests/run_agentos_tests.cpp
        src/AgentEngine/Agent.cpp
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

# Test runner (opcional, compilar com -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build AgentOS test runner" OFF)
if(BUILD_TESTS)
    add_executable(AgentOS_Tests
        src/test_runner.cpp
        src/AgentEngine/Agent.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/ToolEngine/ToolEngine.cpp
        src/ChangeManagement/ChangeManagement.cpp
    )
    target_include_directories(AgentOS_Tests PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_Tests PRIVATE sqlite)
    target_compile_features(AgentOS_Tests PRIVATE cxx_std_20)
endif()
