# Segurança

## Reportando vulnerabilidades

Se você encontrou uma vulnerabilidade de segurança no AgentOS:

1. **Não abra uma issue pública**
2. Envie um email para os mantenedores
3. Inclua:
   - Descrição do problema
   - Passos para reproduzir
   - Impacto potencial
   - Sugestão de correção (se aplicável)

## Áreas críticas

- **Sandbox**: relatar qualquer bypass de isolamento de workspace
- **PolicyEngine**: qualquer falha nas permissões de agentes
- **Emergency Stop**: falha no mecanismo de parada de emergência
- **Auditoria**: qualquer ação não rastreada

## Boas práticas

- Agentes nunca devem acessar sistemas fora do workspace
- Mudanças críticas exigem aprovação do CEO
- Rollback deve ser possível para qualquer alteração
