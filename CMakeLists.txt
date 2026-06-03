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

# Executável Principal JUCE
juce_add_gui_app(AgentOS
    PRODUCT_NAME "AgentOS"
    VERSION 1.0.0
)

target_sources(AgentOS PRIVATE src/main.cpp)

target_link_libraries(AgentOS PRIVATE 
    UI
    AgentEngine 
    EventBus 
    MemoryEngine 
    sqlite
)

add_library(sqlite STATIC libs/sqlite/sqlite3.c)
target_include_directories(sqlite PUBLIC libs/sqlite)
target_link_libraries(AgentOS PRIVATE MemoryEngine sqlite)

# Test runner (opcional, compilar com -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build AgentOS test runner" OFF)
if(BUILD_TESTS)
    add_executable(AgentOS_Tests
        src/test_runner.cpp
        src/AgentEngine/Agent.cpp
        src/EventBus/EventBus.cpp
        src/MemoryEngine/MemoryEngine.cpp
        src/ToolEngine/ToolEngine.cpp
    )
    target_include_directories(AgentOS_Tests PRIVATE include libs/sqlite)
    target_link_libraries(AgentOS_Tests PRIVATE sqlite)
    target_compile_features(AgentOS_Tests PRIVATE cxx_std_20)
endif()
