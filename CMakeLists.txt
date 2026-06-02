cmake_minimum_required(VERSION 3.20)
project(AgentOS VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(include)

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

# Executável Principal
add_executable(AgentOS src/main.cpp)

# (Futuro: Configuração de dependências como JUCE, SQLite, llama.cpp, FAISS, Clang AST)

target_link_libraries(AgentOS PRIVATE AgentEngine EventBus)

add_library(sqlite STATIC libs/sqlite/sqlite3.c)
target_include_directories(sqlite PUBLIC libs/sqlite)
target_link_libraries(AgentOS PRIVATE MemoryEngine sqlite)
