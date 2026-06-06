add_library(ProjectContext STATIC
    ContextBuilder.cpp
    ContextRetriever.cpp
    EmbeddingEngine.cpp
    FileScanner.cpp
    LlamaEmbeddings.cpp
    ProjectScanner.cpp
    PromptComposer.cpp
    SemanticStore.cpp
    SymbolIndexer.cpp
    TextChunker.cpp
    UniversalIndexer.cpp
)
target_include_directories(ProjectContext PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ProjectContext PUBLIC llama)
