add_library(ProjectManager STATIC ProjectManager.cpp)
target_link_libraries(ProjectManager PRIVATE sqlite)
