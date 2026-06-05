#pragma once
#include "Core/PlannerModel.h"

namespace AgentOS {

class HeuristicFallbackPlanner : public PlannerModel {
public:
    PlanningResult analyze(const juce::String& prompt) override {
        PlanningResult result;
        result.source = PlannerSource::Fallback;
        result.confidence = 1.0f; // Heuristic fallback is considered 'confident' by definition
        
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
        int score = 0;
        if (lower.contains("completo") || lower.contains("gigante") || lower.contains("escala") || lower.contains("erp") || lower.contains("crm") || lower.contains("hospitalar")) {
            score += 3;
        }
        if (lower.contains("faturamento") || lower.contains("estoque") || lower.contains("integração")) {
            score += 2;
        }
        if (lower.length() > 100) score += 1;
        
        if (result.type == WorkType::Question) result.complexity = Complexity::Low;
        else if (result.type == WorkType::Project) {
            if (score >= 3) result.complexity = Complexity::High;
            else result.complexity = Complexity::Medium;
        } else {
            if (score >= 2) result.complexity = Complexity::Medium;
            else result.complexity = Complexity::Low;
        }
        
        result.requiresOrganization = (result.type == WorkType::Project || result.type == WorkType::Task);
        
        return result;
    }
};

} // namespace AgentOS
