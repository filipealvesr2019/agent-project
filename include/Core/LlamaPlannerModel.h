#pragma once
#include "Core/PlannerModel.h"
#include "LocalRuntime/LocalRuntimeEngine.h"

namespace AgentOS {

class LlamaPlannerModel : public PlannerModel {
public:
    PlanningResult analyze(const juce::String& prompt) override {
        // Here we would call LocalRuntimeEngine::execute(...)
        // For now, we simulate a JSON response to keep the UI building fast, 
        // while structuring the extraction exactly as requested.
        
        juce::String systemPrompt = R"(
        Analyze the prompt.
        Determine:
        - intent (QUESTION, TASK, PROJECT, DEBUG, SUMMARIZE)
        - domain (SOFTWARE, MARKETING, DATA, RESEARCH, BUSINESS, GENERAL, UNKNOWN)
        - complexity (LOW, MEDIUM, HIGH)
        - confidence (0.0 to 1.0)
        Respond ONLY in JSON format.
        )";
        
        // Simulating the LLM call...
        // auto response = LocalRuntimeEngine::getInstance().execute(1, systemPrompt.toStdString() + prompt.toStdString(), ContextPackage{});
        
        // Mock response representing LLM hallucination limits
        juce::String mockLlmResponse = "Here is the JSON you requested:\n";
        mockLlmResponse += "{\n";
        
        if (prompt.containsIgnoreCase("crie") || prompt.containsIgnoreCase("erp") || prompt.containsIgnoreCase("crm")) {
            mockLlmResponse += "  \"intent\": \"PROJECT\",\n";
            if (prompt.containsIgnoreCase("software") || prompt.containsIgnoreCase("erp")) {
                mockLlmResponse += "  \"domain\": \"SOFTWARE\",\n";
            } else {
                mockLlmResponse += "  \"domain\": \"GENERAL\",\n";
            }
            mockLlmResponse += "  \"complexity\": \"HIGH\",\n";
            mockLlmResponse += "  \"confidence\": 0.95\n";
        } else if (prompt.containsIgnoreCase("quem") || prompt.containsIgnoreCase("o que")) {
            mockLlmResponse += "  \"intent\": \"QUESTION\",\n";
            mockLlmResponse += "  \"domain\": \"GENERAL\",\n";
            mockLlmResponse += "  \"complexity\": \"LOW\",\n";
            mockLlmResponse += "  \"confidence\": 0.88\n";
        } else {
            // Force a low confidence for fallback testing if it's too ambiguous
            mockLlmResponse += "  \"intent\": \"UNKNOWN\",\n";
            mockLlmResponse += "  \"domain\": \"UNKNOWN\",\n";
            mockLlmResponse += "  \"complexity\": \"LOW\",\n";
            mockLlmResponse += "  \"confidence\": 0.30\n";
        }
        
        mockLlmResponse += "}";
        
        return extractJSON(mockLlmResponse);
    }
    
private:
    PlanningResult extractJSON(const juce::String& rawResponse) {
        PlanningResult result;
        result.source = PlannerSource::LLM;
        
        int startIdx = rawResponse.indexOfChar('{');
        int endIdx = rawResponse.lastIndexOfChar('}');
        
        if (startIdx == -1 || endIdx == -1 || startIdx > endIdx) {
            result.confidence = 0.0f;
            result.fallbackReason = "Invalid JSON structure from LLM";
            return result;
        }
        
        juce::String jsonStr = rawResponse.substring(startIdx, endIdx + 1);
        auto parsed = juce::JSON::parse(jsonStr);
        
        if (parsed.isVoid() || !parsed.isObject()) {
            result.confidence = 0.0f;
            result.fallbackReason = "Failed to parse JSON string";
            return result;
        }
        
        auto* obj = parsed.getDynamicObject();
        
        result.confidence = obj->getProperty("confidence").toString().getFloatValue();
        
        juce::String intentStr = obj->getProperty("intent").toString();
        if (intentStr == "PROJECT") result.type = WorkType::Project;
        else if (intentStr == "QUESTION") result.type = WorkType::Question;
        else if (intentStr == "TASK") result.type = WorkType::Task;
        else result.type = WorkType::Custom;
        
        juce::String domainStr = obj->getProperty("domain").toString();
        if (domainStr == "SOFTWARE") result.domain = Domain::Software;
        else if (domainStr == "MARKETING") result.domain = Domain::Marketing;
        else if (domainStr == "DATA") result.domain = Domain::Data;
        else if (domainStr == "RESEARCH") result.domain = Domain::Research;
        else if (domainStr == "BUSINESS") result.domain = Domain::Business;
        else result.domain = Domain::General;
        
        juce::String complexStr = obj->getProperty("complexity").toString();
        if (complexStr == "HIGH") result.complexity = Complexity::High;
        else if (complexStr == "MEDIUM") result.complexity = Complexity::Medium;
        else result.complexity = Complexity::Low;
        
        result.requiresOrganization = (result.type == WorkType::Project || result.type == WorkType::Task);
        
        return result;
    }
};

} // namespace AgentOS
