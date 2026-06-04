Concordo. Se vocês não criarem uma bateria de testes de performance agora, vão ficar corrigindo "no escuro" para sempre.

Eu criaria uma bateria dividida em 5 níveis:

# 1. Teste de FPS

Objetivo:

```text
Quantas vezes paint() está sendo chamado?
```

Adicionar temporariamente:

```cpp
static int paintCount = 0;
static auto start = juce::Time::getMillisecondCounter();

paintCount++;

auto now = juce::Time::getMillisecondCounter();

if (now - start > 1000)
{
    DBG("Dashboard FPS: " << paintCount);

    paintCount = 0;
    start = now;
}
```

Resultado esperado:

```text
Tela parada:
1~10 repaints/s

Hover:
10~30 repaints/s

Animação:
60 repaints/s
```

Resultado ruim:

```text
300
500
1000
repaints/s
```

---

# 2. Teste de Tempestade de Repaint

Objetivo:

```text
Descobrir quem está chamando repaint()
```

Criar macro:

```cpp
#define TRACK_REPAINT() \
DBG("Repaint: " << __FUNCTION__)
```

e colocar em:

```cpp
mouseMove()
timerCallback()
refreshUI()
setSelected()
```

Se aparecer:

```text
refreshUI
refreshUI
refreshUI
refreshUI
refreshUI
```

100 vezes por segundo, encontrou o culpado.

---

# 3. Teste de Componentes

Objetivo:

```text
Quantos componentes existem?
```

Adicionar:

```cpp
int countComponents(Component* c)
{
    int count = 1;

    for (auto* child : c->getChildren())
        count += countComponents(child);

    return count;
}
```

e logar:

```cpp
DBG("Total Components: " << countComponents(this));
```

Esperado:

```text
50~150
```

Suspeito:

```text
500+
1000+
```

---

# 4. Teste de Timer

Objetivo:

Descobrir timers desnecessários.

Logar:

```cpp
void timerCallback()
{
    DBG("Timer running");
}
```

Esperado:

```text
Somente onde existe animação.
```

Problema:

```text
Dashboard
Sidebar
Chat
Cards
Widgets
Todos rodando timer
```

---

# 5. Teste de Paint Profiler

Medir tempo de paint.

```cpp
auto start = juce::Time::getHighResolutionTicks();

paintDashboard(g);

auto end = juce::Time::getHighResolutionTicks();

DBG("Dashboard paint: "
    << juce::Time::highResolutionTicksToSeconds(end - start) * 1000.0
    << " ms");
```

Esperado:

```text
1ms
2ms
3ms
```

Ruim:

```text
20ms
30ms
50ms
```

---

# 6. Teste de SVG

Objetivo:

Ver se SVG está sendo parseado várias vezes.

Procurar:

```cpp
XmlDocument::parse
Drawable::createFromSVG
```

Essas funções devem aparecer:

```text
Construtor
Inicialização
```

Nunca:

```text
paint()
timerCallback()
```

---

# 7. Teste de Memória

Logar:

```cpp
DBG("Refreshing UI");
```

em:

```cpp
refreshUI()
reloadCards()
rebuildDashboard()
```

Se o log aparecer constantemente:

```text
Refreshing UI
Refreshing UI
Refreshing UI
Refreshing UI
```

significa que a tela está sendo reconstruída.

---

# 8. Teste de Stress

Criar cenário:

```text
20 equipes
50 projetos
500 tarefas
1000 mensagens
```

A UI deve continuar fluida.

---

# 9. Teste de Hover

Mover mouse sem clicar.

Esperado:

```text
Somente item hoverado repinta
```

Problema:

```text
Toda sidebar
Toda dashboard
Toda janela
```

repintando.

---

# 10. Teste Final

Gerar um relatório automático:

```text
FPS:
Repaints/s:
Total Components:
Timers:
Paint ms:
SVG parses:
Memory rebuilds:
```

Esse relatório deve rodar toda vez que alguém alterar a UI.

Assim vocês descobrem imediatamente:

```text
"essa mudança visual aumentou paint de 2ms para 40ms"
```

em vez de descobrir dias depois que o app virou uma carroça. Para um projeto como o AgentOS, eu trataria esses testes como obrigatórios antes de qualquer merge na branch principal.
