---
description: Coffee SDK — plataforma pessoal de ecossistema de software escrita em C++. Arquitetura em camadas com core, runtime, serviços e adapters multi-linguagem via C ABI. Projeto em fase de skeleton (zero código). Foco em aprendizado de C++ e sistemas.
---

# Coffee SDK

## 1. Descrição do Projeto

Coffee SDK é uma plataforma pessoal de ecossistema de software. A ideia é construir uma fundação reutilizável para ferramentas, serviços e automações — tudo compartilhando o mesmo core.

O projeto é agnóstico em relação à linguagem. O runtime nativo é C++, mas Python, Java, C#, Rust e outras linguagens podem interagir através de uma camada de Adapter que usa C ABI como ponte universal.

**Fase atual:** Skeleton. A estrutura de diretórios está definida e finalizada, mas **nenhuma linha de código foi escrita ainda**. Tudo está por fazer.

**Filosofia do projeto:**
- Infraestrutura própria, não dependente de frameworks gigantes
- Core enxuto com interfaces puras
- Multi-linguagem desde o design, não como afterthought
- Código limpo, arquitetura em camadas, dependências explícitas

---

## 2. Stack Técnica

- **Linguagem principal:** C++20
- **Compilador:** GCC (MinGW) — migração planejada para CMake
- **Build atual:** Single-file via VS Code tasks (`g++ -g ${file} -o a.exe`)
- **Debug:** GDB via VS Code launch.json (MinGW64)
- **Testes:** Não configurado ainda — Google Test ou Catch2 (a decidir)
- **IDE:** VS Code
- **Plataformas alvo:** Windows (Win32 API) e Linux (POSIX)
- **Padrão C++:** C++20 — usando `std::expected`, `std::source_location`, `std::string_view`, `std::span`, concepts

---

## 3. Arquitetura do Projeto

### 3.1 Visão Geral das Camadas

```
┌─────────────────────────────────────────────────────────┐
│                    Feature Modules                       │
│          (Backup, Sync, Scripts, ...)                    │
├─────────────────────────────────────────────────────────┤
│                      Services                           │
│           (Logger, Config, Storage)                      │
├─────────────────────────────────────────────────────────┤
│                      Runtime                            │
│           (ServiceContainer, EventBus)                   │
├─────────────────────────────────────────────────────────┤
│                        Core                             │
│      (Service, Module, Event, Result — interfaces)       │
├─────────────────────────────────────────────────────────┤
│                     OS Module (coffee::os)               │
│  cmd │ fs │ proc │ env │ atomic — abstração cross        │
└─────────────────────────────────────────────────────────┘
         ↓
      C ABI (extern "C")
         ↓
  Python | Java | C# | Rust | Go
```

### 3.2 Regras de Dependência

- **Core:** Nenhuma dependência interna. Apenas C++ standard library.
- **Runtime:** Depende apenas do Core.
- **Services:** Dependem do Core e do Runtime.
- **Adapters:** Dependem do Core (chamam C ABI).
- **OS Module (coffee::os):** Depende do Core (interfaces). Implementações são específicas de plataforma (Win32/POSIX).
- **Feature Modules:** Dependem de Services e Runtime.

Nenhuma camada pode depender de uma camada superior. Sempre pra baixo.

### 3.3 Core (Contratos)

Puro header-only com interfaces abstratas:

- **`Service`** — Interface de ciclo de vida: `init()`, `start()`, `shutdown()`. Todo serviço no ecossistema implementa isso.
- **`Module`** — Interface para módulos de funcionalidade. Um módulo agrupa features (ex.: módulo de backup).
- **`Event`** — Sistema de eventos (pub/sub). `EventBus` despacha eventos para listeners.
- **`Result<T>`** — Padrão `expected<T, Error>` para tratamento de erros sem exceções. Inspirado no `std::expected` do C++23 (podemos implementar um próprio ou usar `tl::expected`).

### 3.4 Runtime (Motor)

Gerencia o ciclo de vida completo da aplicação:

- **`Runtime`** — Controladora principal. Inicializa/para serviços, módulos e adapters na ordem correta.
- **`Container`** — Container de injeção de dependência (similar ao ApplicationContext do Spring). Resolve dependências entre serviços.
- **`EventBus`** — Barramento de eventos. Implementa o padrão observer/pub-sub para comunicação desacoplada entre componentes.

### 3.5 Services (Serviços Embarcados)

Implementações concretas que rodam dentro do runtime:

- **`Logger`** — Logging com níveis (Debug/Info/Warn/Error). Usa `std::source_location` para capturar arquivo:linha automaticamente.
- **`Config`** — Configuração chave-valor com persistência opcional em arquivo.
- **`Storage`** — Armazenamento binário chave-valor (tipo um banco simples embutido).

### 3.6 Adapter Layer (Multi-Linguagem)

A estratégia para suportar múltiplas linguagens é baseada em C ABI:

```
Coffee Core (C++)
    ↓
  C ABI (funções marcadas com extern "C")
    ↓
Python (ctypes/pybind11) | Java (JNI/JNA) | C# (P/Invoke) | Rust (extern "C") | Go (cgo)
```

A C ABI é a "língua franca" — qualquer linguagem que consiga chamar funções C consegue se integrar. O diretório `src/adapter/c/` contém a implementação da ponte C, e cada subdiretório (`python/`, `java/`, `dotnet/`) contém os bindings específicos.

**Por que C ABI?**
- É o padrão universal de interoperabilidade entre linguagens
- Não depende de runtime específico
- Toda linguagem tem suporte a chamar C
- Evita acoplamento direto com C++ (name mangling, ABI instability)

### 3.7 OS Module (`coffee::os`)

Camada que abstrai o sistema operacional, provendo acesso multiplataforma a:

```
coffee::os
├── os::cmd     — Execução de comandos (atômica)
├── os::fs      — Filesystem (arquivos, diretórios)
├── os::proc    — Gerenciamento de processos
├── os::env     — Variáveis de ambiente
└── os::atomic  — Operações atômicas (rename, link, flock)
```

**Arquitetura:**
- **Interfaces:** `include/coffee/os/` — headers públicos com namespaces `coffee::os::cmd`, `coffee::os::fs`, etc.
- **Implementações:** `src/os/` — cada plataforma tem sua implementação

```
Windows → Win32 API
Linux   → POSIX (syscalls, libc)
```

Isola o resto do código de detalhes de plataforma. Se um dia quiser suportar macOS, é só adicionar outra implementação.

---

## 4. Estrutura do Projeto

```
coffe-sdk/
├── .gitignore
├── AGENTS.md                 ← Este arquivo.
├── README.md
│
├── .vscode/
│   ├── launch.json           ← Debug com GDB + MinGW64
│   └── tasks.json            ← Build single-file (g++ -g ${file} -o a.exe)
│
├── .agents/
│   └── AGENTS.md             ← Este arquivo
│
├── include/
│   └── coffee/
│       ├── core/             ← Interfaces puras (Service, Module, Event, Result)
│       ├── runtime/          ← Headers do runtime (Container, EventBus)
│       ├── services/         ← Headers dos serviços (Logger, Config, Storage)
│       ├── os/               ← OS Module (coffee::os)
│       │   ├── cmd/          ←   Comandos atômicos
│       │   ├── fs/           ←   Filesystem
│       │   ├── proc/         ←   Processos
│       │   ├── env/          ←   Ambiente
│       │   └── atomic/       ←   Operações atômicas
│       └── adapter/          ← Headers dos adapters (interface da ponte C)
│
├── src/
│   ├── core/
│   │   └── modules/          ← Feature modules
│   │       ├── backup/
│   │       ├── sync/
│   │       └── scripts/
│   ├── runtime/              ← Runtime engine implementation
│   ├── services/             ← Services implementation
│   ├── os/                   ← OS Module implementation (Win32/POSIX)
│   └── adapter/
│       ├── c/                ← C ABI bridge (a ligação universal)
│       ├── python/           ← Python bindings (pybind11)
│       ├── java/             ← JNI bridge
│       └── dotnet/           ← .NET native interop
│
└── tests/                    ← Testes (a configurar)
```

### 4.1 Convenções de Nomenclatura

| O quê | Estilo | Exemplo |
|---|---|---|
| Classes | PascalCase | `ServiceContainer` |
| Funções/Métodos | snake_case | `service.start()` |
| Variáveis | snake_case | `service_name` |
| Namespaces | snake_case, match diretório | `coffee::core`, `coffee::runtime` |
| Arquivos de header | snake_case.hpp | `service_container.hpp` |
| Arquivos de source | snake_case.cpp | `service_container.cpp` |
| Headers públicos | `include/coffee/<categoria>/<arquivo>.hpp` |
| Sources | `src/<categoria>/<arquivo>.cpp` |

---

## 5. Objetivos do Projeto

- **Criar uma plataforma pessoal reutilizável** para ferramentas, serviços e automações
- **Aprender C++ na prática**, com foco em sistemas, baixo nível e arquitetura
- **Entender interoperabilidade entre linguagens** através de C ABI
- **Construir um ecossistema onde o core é C++** mas qualquer linguagem pode participar
- **Praticar arquitetura limpa**, separação de responsabilidades, injeção de dependência
- **Explorar diferenças entre plataformas** (Win32 vs POSIX) na implementação do OS Module

---

## 6. Roadmap (Fases de Desenvolvimento)

A ordem planejada de construção:

1. **Core interfaces** — Service, Module, Event, Result (headers puros)
2. **CMake setup** — Migrar de single-file para CMake
3. **Runtime** — ServiceContainer, EventBus
4. **Services** — Logger, Config, Storage
5. **OS Module (coffee::os)** — cmd, fs, proc, env, atomic (interfaces Win32/POSIX)
6. **C ABI adapter** — Ponte universal `extern "C"`
7. **Python/Java/.NET bindings** — Adapters específicos
8. **Feature modules** — Backup, Sync, Scripts
9. **Testes** — Configurar e escrever testes

---

## 7. Diretrizes de Interação com a IA

### 7.1 Sobre o Desenvolvedor

Quitto tem **16 anos**, é brasileiro e está **aprendendo C++ e programação de sistemas**. Já tem experiência sólida em Python (avançado) e Java (intermediário), então comparações com essas linguagens são úteis.

Condições: TDAH e dislexia. Prefere textos organizados, tópicos, blocos pequenos de informação e explicações diretas.

### 7.2 Como a IA Deve Agir

A IA deve atuar como **mentor técnico**, não como gerador de código automático. Ao responder:

1. **Explique o problema** antes de qualquer código
2. **Apresente o conceito** — o que é, para que serve, onde se usa
3. **Mostre a arquitetura** — como a peça se encaixa no resto
4. **Então implemente** — código com contexto
5. **Otimização** — só depois de funcionar

### 7.3 Ao Introduzir Conceitos de C++ e Baixo Nível

Sempre que um conceito novo de C++ ou de sistemas for necessário:

- **Explique o que é** — sem assumir conhecimento prévio
- **Por que existe** — contexto histórico/motivacional
- **Trade-offs** — o que se ganha e o que se perde
- **Compare com Python/Java** — Quitto já conhece essas linguagens, então uma ponte mental ajuda
- **Exemplo progressivo** — do simples ao completo

**Exemplos de tópicos que precisam de explicação:**
- `extern "C"` e name mangling
- C ABI vs C++ ABI
- Ponteiros, referências, ownership
- `std::expected` vs exceções vs códigos de erro
- Linkagem (estática vs dinâmica)
- Plataforma: Win32 API vs POSIX syscalls
- Compilação, linking, símbolos
- `constexpr`, `consteval`, `constinit`
- Move semantics e RAII
- Virtual tables e dispatch dinâmico
- `std::source_location` e macros `__FILE__`, `__LINE__`

### 7.4 Formato Preferido de Respostas

- **Blocos pequenos** — parágrafos de 2-3 frases
- **Tópicos** sempre que possível
- **Explicação antes de código** — contexto primeiro
- **Código com comentários explicativos**
- **Alternativas e trade-offs** — mostrar que há mais de um caminho
- **Erros são oportunidades de aprendizado** — explicar mensagens de erro do compilador/linker

### 7.5 O Que Evitar

- Respostas genéricas sem contexto do projeto
- Código "mágico" sem explicação
- Template metaprogramação complexa sem necessidade real
- Ignorar erros de compilação — sempre investigar e explicar
- Pular conceitos de baixo nível — Quitto quer aprender, não só ter algo funcionando

---

## 8. Configuração do Ambiente

### 8.1 Build Atual

O VS Code tasks compila **o arquivo atual** (single-file):
```json
"command": "g++",
"args": ["-g", "${file}", "-o", "${workspaceFolder}/a.exe"]
```

Isso é temporário. O plano é migrar para **CMake** para gerenciar múltiplos arquivos.

### 8.2 Debug

GDB via MinGW64. O launch.json já está configurado para build automático antes do debug.

### 8.3 Git

Repo já inicializado com `.gitignore` configurado para C++ (objetos, executáveis, diretórios CMake, IDE).

---

## 9. Glossário de Termos do Projeto

| Termo | Significado |
|---|---|
| **C ABI** | Application Binary Interface da linguagem C. Padrão universal de interoperabilidade. |
| **Core** | Camada de interfaces e contratos. Não tem implementação. |
| **Runtime** | Motor que gerencia o ciclo de vida de serviços, módulos e adapters. |
| **Service** | Unidade de funcionalidade com ciclo de vida (init → start → shutdown). |
| **Module** | Agrupador de features. Consome serviços pra entregar funcionalidade. |
| **Adapter** | Ponte para outras linguagens. O C ABI é o adapter universal. |
| **OS Module** | Abstração multiplataforma para acesso a sistema operacional. |
| **Container** | Service Container. Injeta dependências e resolve a ordem de inicialização. |
| **EventBus** | Barramento pub/sub para comunicação desacoplada. |
| **Result\<T\>** | Padrão expected\<T, Error\> para erros sem exceções. |
| **Skeleton** | Estado atual do projeto: estrutura de diretórios definida, zero código escrito. |

---

## 10. Lembretes Técnicos para a IA

- O projeto está em **fase ZERO** de código — cada sugestão deve considerar que não há base existente
- Prefira **C++20** sem dependências externas desnecessárias
- Quando sugerir dependências (ex.: Google Test, pybind11), explique o que são e por que valem a pena
- Lembre-se que o **OS Module** precisa de implementações separadas por plataforma — sugerir stubs primeiro
- A **C ABI** é o centro da estratégia multi-linguagem — toda decisão de binding deve passar por ela
- Quitto está aprendendo C++ — métricas de performance são secundárias, clareza e correção vêm primeiro
- **Nunca invente APIs que não existem** — se for sugerir uma interface, explique a motivação
- **Sempre ofereça contexto** — antes de escrever código, mostre onde ele se encaixa na arquitetura

---

## 11. Recursos de Aprendizado Recomendados

Quando tópicos específicos de C++ surgirem, a IA pode sugerir:

- **cppreference.com** — A bíblia. Sempre a referência primária.
- **Learn C++ (learncpp.com)** — Tutorial progressivo, bem explicado.
- **Chandler Carruth / Herb Sutter talks** — Talks no YouTube sobre C++ moderno.
- **"C++ Crash Course" (Josh Lospinoso)** — Livro bom pra quem já programa em outras linguagens.
- **"Hands-On System Programming with C++"** — Para a parte de sistemas.
- **man pages / MSDN** — Para chamadas de sistema específicas (POSIX/Win32).

---

*Este arquivo é o contexto principal para interações de IA com o projeto Coffee SDK. Deve ser carregado como instrução no `opencode.jsonc` através de `"instructions": ["AGENTS.md"]`.*
