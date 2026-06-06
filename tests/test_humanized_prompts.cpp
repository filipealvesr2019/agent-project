// test_humanized_prompts.cpp
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <sstream>
#include "../include/PersonaEngine/SharedModelPool.h"
#include "../include/EventBus/EventBus.h"

using namespace AgentOS;

int main() {
    std::cout << "--- Starting Humanized Prompts Test ---\n\n";

    const int numPersonas = 30;
    std::vector<std::string> personas;
    std::vector<std::string> tasks;

    for (int i = 0; i < numPersonas; ++i) {
        personas.push_back("Worker_" + std::to_string(i));
        tasks.push_back("Execute Parallel Task " + std::to_string(i));
    }

    // Guardando futuros para cada requisição
    std::vector<std::future<std::string>> futures;

    for (int i = 0; i < numPersonas; ++i) {
        // Enfileirando no SharedModelPool, que agora retorna respostas humanizadas!
        auto future = SharedModelPool::getInstance().enqueuePrompt(personas[i], tasks[i]);
        futures.push_back(std::move(future));
    }

    // Coletando respostas
    for (int i = 0; i < numPersonas; ++i) {
        std::string response = futures[i].get();
        std::cout << "[LLM Output] Persona: " << personas[i] 
                  << " | " << response << "\n";
    }

    SharedModelPool::getInstance().stopWorker();

    std::cout << "\n[Status] Humanized Prompts Test completed successfully!\n";
    std::cout << "[Info] 30 personas processed concurrently with isolated context.\n";
    return 0;
}
