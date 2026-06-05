#include "Core/CEOPlanner.h"
#include "Core/ComplexityEngine.h"

namespace AgentOS {

juce::String PlanningResult::getTypeStr() const {
    switch(type) {
        case WorkType::Question: return "Question";
        case WorkType::Task: return "Task";
        case WorkType::Project: return "Project";
        default: return "Custom";
    }
}

juce::String PlanningResult::getDomainStr() const {
    switch(domain) {
        case Domain::Software: return "Software";
        case Domain::Marketing: return "Marketing";
        case Domain::Data: return "Data";
        case Domain::Research: return "Research";
        case Domain::Business: return "Business";
        default: return "General";
    }
}

juce::String PlanningResult::getComplexityStr() const {
    switch(complexity) {
        case Complexity::Low: return "Low";
        case Complexity::Medium: return "Medium";
        case Complexity::High: return "High";
        default: return "Low";
    }
}

PlanningResult CEOPlanner::analyze(const juce::String& prompt) {
    PlanningResult result;
    juce::String lower = prompt.toLowerCase();
    
    // 1. Intent Analysis
    if (lower.startsWith("quem ") || lower.startsWith("o que ") || lower.startsWith("qual ") || 
        lower.startsWith("como ") || lower.startsWith("onde ") || lower.startsWith("por que ") || 
        lower.startsWith("explique") || lower.startsWith("quanto")) {
        result.type = WorkType::Question;
    } else if (lower.contains("debugar") || lower.contains("resuma") || lower.contains("corrija") || lower.contains("analise") || lower.contains("ajude")) {
        result.type = WorkType::Task;
    } else if (lower.contains("crie") || lower.contains("construa") || lower.contains("desenvolva") || lower.contains("projeto") || lower.contains("sistema") || lower.contains("erp") || lower.contains("crm")) {
        result.type = WorkType::Project;
    } else {
        result.type = WorkType::Custom;
    }

    // 2. Domain Detection
    if (lower.contains("software") || lower.contains("código") || lower.contains("crm") || lower.contains("erp") || lower.contains("react") || lower.contains("next.js") || lower.contains("saas") || lower.contains("app") || lower.contains("site")) {
        result.domain = Domain::Software;
    } else if (lower.contains("marketing") || lower.contains("campanha") || lower.contains("venda") || lower.contains("landing page") || lower.contains("anúncio")) {
        result.domain = Domain::Marketing;
    } else if (lower.contains("dados") || lower.contains("análise") || lower.contains("dataset") || lower.contains("csv") || lower.contains("financeiro")) {
        result.domain = Domain::Data;
    } else if (lower.contains("pesquisa") || lower.contains("artigo") || lower.contains("pdf") || lower.contains("resumo")) {
        result.domain = Domain::Research;
    } else if (lower.contains("negócios") || lower.contains("empresa") || lower.contains("plano")) {
        result.domain = Domain::Business;
    } else {
        result.domain = Domain::General;
    }

    // 3. Complexity Estimation
    result.complexity = ComplexityEngine::estimate(prompt, result.type);
    
    // 4. Tasks generation
    if (result.domain == Domain::Software) {
        result.tasks.add("Frontend");
        result.tasks.add("Backend");
        if (result.complexity == Complexity::High) {
            result.tasks.add("Database");
            result.tasks.add("QA");
            result.tasks.add("DevOps");
        }
    } else if (result.domain == Domain::Marketing) {
        result.tasks.add("Copywriting");
        result.tasks.add("Design");
        if (result.complexity == Complexity::High) {
            result.tasks.add("SEO");
            result.tasks.add("Social Media");
        }
    } else if (result.domain == Domain::Research) {
        result.tasks.add("Pesquisa");
        if (result.complexity != Complexity::Low) {
            result.tasks.add("Revisao");
        }
    } else {
        result.tasks.add("Execução");
    }
    
    // 5. Summary
    juce::StringArray words;
    words.addTokens(prompt, " ,.", "\"");
    if (words.size() > 2) {
        result.summary = words[0] + " " + words[1] + " " + words[2] + "...";
    } else {
        result.summary = "Nova Operação";
    }

    return result;
}

} // namespace AgentOS
