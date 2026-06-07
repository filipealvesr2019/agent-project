add_library(ProjectContext STATIC
    ContextBuilder.cpp
    ContextRetriever.cpp
    EmbeddingEngine.cpp
    FileScanner.cpp
    FileSummarizer.cpp
    FileSummaryStore.cpp
    LlamaEmbeddings.cpp
    ProjectScanner.cpp
    PromptComposer.cpp
    SemanticStore.cpp
    SymbolIndexer.cpp
    TextChunker.cpp
    UniversalIndexer.cpp
    CodeGraph.cpp
    Reranker.cpp
)
target_include_directories(ProjectContext PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ProjectContext PUBLIC llama sqlite)
target_link_libraries(ProjectContext PRIVATE LocalRuntime)
