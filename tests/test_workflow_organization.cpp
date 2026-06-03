#include <iostream>
#include <cassert>
#include "WorkflowEngine/WorkflowEngine.h"
#include "OrganizationEngine/OrganizationEngine.h"
#include "AgentEngine/Agent.h"


using namespace AgentOS;

void testOrganizationWorkflow() {
    std::cout << "--- Inciando Teste: Organization Workflow ---\n";

    auto& orgEngine = OrganizationEngine::getInstance();
    auto& wfEngine = WorkflowEngine::getInstance();

    // 1. Criar Organização
    bool orgCreated = orgEngine.createOrganization("FutureCorp", "A test organization");
    assert(orgCreated);

    // 2. Criar Departamentos
    Department devDept{"Development", {"Alice"}, {"Bob", "Charlie"}};
    Department qaDept{"Quality Assurance", {"Dave"}, {"Eve"}};
    orgEngine.addDepartment("FutureCorp", devDept);
    orgEngine.addDepartment("FutureCorp", qaDept);

    auto org = orgEngine.getOrganization("FutureCorp");
    assert(org.departments.size() == 2);

    // 3. Workflow Objective atrelado a Organization
    int objId = wfEngine.createObjective("Lançar AgentOS", "Completar fase 2", "Alice", "FutureCorp");
    assert(objId > 0);

    auto objs = wfEngine.getObjectivesForOrganization("FutureCorp");
    assert(!objs.empty());
    assert(objs[0].organization == "FutureCorp");

    // 4. Criar e designar tasks por departamento
    int devTaskId = wfEngine.createTask("Programar UI", "Implementar Canvas", "Bob", "Alice", objId, 0, WorkflowPriority::High, "FutureCorp", "Development");
    int qaTaskId = wfEngine.createTask("Testar UI", "Validar Canvas", "Eve", "Dave", objId, 0, WorkflowPriority::Medium, "FutureCorp", "Quality Assurance");

    auto devTasks = wfEngine.getTasksForDepartment("FutureCorp", "Development");
    assert(devTasks.size() == 1);
    assert(devTasks[0].id == devTaskId);

    auto qaTasks = wfEngine.getTasksForDepartment("FutureCorp", "Quality Assurance");
    assert(qaTasks.size() == 1);
    assert(qaTasks[0].id == qaTaskId);

    // 5. Associar Agent a Department e Organization
    Agent bob("Bob", "Developer", "Development", "FutureCorp");
    assert(bob.getOrganization() == "FutureCorp");
    assert(bob.getDepartment() == "Development");

    std::cout << "--- Sucesso: Organization Workflow ---\n";
}

int main() {
    testOrganizationWorkflow();
    return 0;
}
