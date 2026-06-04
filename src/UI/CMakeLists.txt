# CMakeLists for UI
add_library(UI STATIC
    UI.cpp
    DashboardComponent.cpp
    SystemMonitorComponent.cpp
    AgentListComponent.cpp
    SidebarComponent.cpp
    LogViewerComponent.cpp
    CreateAgentDialog.cpp
    ModelManagerDialog.cpp
    ProjectPanelComponent.cpp
    CognitiveDashboardComponent.cpp
    SystemMonitorComponent.cpp
    WorkflowEditor/WorkflowEditorComponent.cpp
    MemoryVisualization/MemoryVisualizationComponent.cpp
)
target_include_directories(UI PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(UI PUBLIC AgentOS_Icons
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_events
    juce::juce_core
    juce::juce_data_structures
    AgentEngine
    EventBus
    MemoryEngine
    GovernanceEngine
    Sandbox
    ModelManager
    ProjectManager
    OrganizationEngine
    ReasoningTimelineEngine
    Cognitive
)
