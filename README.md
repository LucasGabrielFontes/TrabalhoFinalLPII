# Arquitetura proposta

```bash
TrabalhoFinalLPII/
├── README.md
├── CMakeLists.txt
├── Makefile                # opcional, chama cmake
├── docs/
│   ├── arquitetura.md
│   ├── diagramas/
│   │   ├── seq_cliente_servidor.png
│   │   └── componentes.png
│   └── relatorio_ia.md     # rascunho do relatório com prompts usados
├── include/
│   ├── server.hpp
│   ├── client_handler.hpp
│   ├── threadsafe_queue.hpp
│   ├── tslogger.hpp        # wrapper mínimo para libtslog
│   └── common.hpp
├── src/
│   ├── main_server.cpp
│   ├── server.cpp
│   ├── client_handler.cpp
│   ├── threadsafe_queue.cpp   # se necessário (templated)
│   ├── tslogger.cpp
│   └── util.cpp
├── third_party/
│   └── libtslog/           # instruções para instalar/usar em docs; git-submodule opcional
├── scripts/
│   ├── run_server.sh
│   ├── run_client.sh
│   ├── spawn_clients.sh    # script para simular N clientes (etapa 2)
│   └── make_release.sh
├── tests/
│   ├── stress_test.py      # script em python para criar conexões concorrentes
│   └── unit_tests/         # testes unitários (gtest ou doctest)
├── examples/
│   ├── client_simple.cpp   # cliente CLI minimal
│   └── client_interactive.cpp
└── README_DEV.md
```

# Arquivos-chave e responsabilidades

## include/common.hpp

    - Tipos compartilhados, constantes (portas, tamanhos), utilitários de logging/erro.
    - Ex.: using socket_t = int; (ou SOCKET no Windows).

## include/tslogger.hpp + src/tslogger.cpp

    - Wrapper simples sobre a libtslog exigida pela especificação.
    - Funções: tslog_init(path, level), tslog_log(level, fmt, ...), tslog_close().
    - Essa camada facilita testar sem expor diretamente a lib tslog em todo o projeto.

## include/threadsafe_queue.hpp

    - Implementa ThreadSafeQueue<T> (monitor encapsulado): fila protegida com std::mutex + std::condition_variable.
    - Métodos: push(T), bool try_pop(T&), wait_and_pop(T&), size().
    - Usada para enfileirar mensagens a serem retransmitidas ou para comunicação interna.

## include/server.hpp + src/server.cpp

Classe Server (encapsula a lógica TCP):

    - Responsabilidades:

        - Abrir listen() socket (porta configurável).
        - Aceitar conexões (thread accept ou loop principal).
        - Criar ClientHandler (thread por cliente) ou enfileirar sockets em um thread-pool.
        - Manter std::vector<std::shared_ptr<ClientHandler>> clients; — protegido por std::mutex clients_mutex.
        - Funções administrativas: broadcast(Message), remove_client(id), get_stats().

    -Proteções:

        - clients_mutex para adicionar/remover/navegar lista de clientes.
        - std::counting_semaphore (ou lógica com condvar) para limitar conexões simultâneas (requisito de semáforo/slot).

## include/client_handler.hpp + src/client_handler.cpp

Classe ClientHandler:

    - Criada por Server para cada cliente.
    - Tem sua própria std::thread que faz loop de recv/process/send.
    Ao receber mensagem, coloca em Server::message_queue (uma ThreadSafeQueue<Message>) ou chama server->broadcast(...).
    - Usa mutex interno só se tiver estado mutável (ex.: buffer).
    - eve detectar disconnects e notificar Server para remoção.

## src/main_server.cpp

    - Parse de CLI (porta, max-clients, log path).
    - Inicializa tslog, constrói Server, cria thread de administração (CLI de controle: comandos de parar, estatísticas).
    - Registra handlers de sinal (SIGINT/SIGTERM) para shutdown gracioso.

## examples/client_simple.cpp

    - CLI simples que conecta, manda linha por stdin, imprime mensagens recebidas.
    - Útil para testes manuais e entrega da etapa 2.

# Modelo de threads e sincronização

1. Thread principal

    - Cria socket listen(), roda loop de accept() ou delega a um accept thread.
    - Cria ClientHandler por conexão (thread por cliente) ou coloca socket em thread-pool.

2. ClientHandler thread (por cliente)

    - Lê do socket (blocking read).
    - Ao receber mensagem: cria Message e chama server->enqueue_message(msg) ou server->broadcast(msg).
    - Envia mensagens ao cliente (pode ter fila de saída própria ou chamada direta ao send).

3. Broadcast / Dispatcher

    - Opção A (simples): broadcast() chama, sob clients_mutex, send() em cada cliente.

        - Prós: simples; Con: envio pode bloquear — recomenda usar sockets não-blocantes ou enviar numa thread de saída por cliente.

    - Opção B (recomendado para robustez): message_queue global (ThreadSafeQueue<Message>) + Dispatcher thread que consome e envia para todos clients.

        - Assim o ClientHandler só enfileira (rápido), evitando bloqueio.
        - Dispatcher faz iteração sob clients_mutex e send() com tratamento de EAGAIN/TIMEOUT.

4. Sincronização

    - clients_mutex (std::mutex) para lista de clientes.
    - ThreadSafeQueue (monitora): std::mutex + std::condition_variable.
    - std::counting_semaphore (C++20) ou contador + condvar para limitar número de conexões (requisito do trabalho).
    - Evitar deadlocks: sempre adquirir mutexes numa ordem estável (ex.: primeiro clients_mutex, depois client->out_mutex).

5. Recursos & RAII

    - Usar std::unique_ptr/shared_ptr para handlers.
    - Encapsular socket num objeto que fecha no destructor (SocketWrapper).

# Logging concorrente (libtslog)

    - Inicializar uma única instância global via tslog_init() em main.
    - Não chame printf diretamente para mensagens de log; sempre TSLog::log(...).
    - O wrapper tslogger deve ser thread-safe (a lib exige; caso contrário, proteja com mutex internamente).
    - Logar: conexões/disconexões, mensagens broadcast, erros, métricas (latência, mensagens/segundo).

# CMake mínimo (esqueleto CMakeLists.txt)

```bash
cmake_minimum_required(VERSION 3.15)
project(TrabalhoFinalLPII LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

include_directories(include)
add_subdirectory(third_party/libtslog) # opcional, se tiver CMake

add_executable(chat_server
  src/main_server.cpp
  src/server.cpp
  src/client_handler.cpp
  src/tslogger.cpp
  src/util.cpp
)

target_link_libraries(chat_server PRIVATE tslog) # se tslog exporta target
```

(Detalhe: se usar libtslog instalada via pkg-config, faça find_package() ou pkg_check_modules.)

# Scripts úteis e automações

## scripts/run_server.sh

Executa build/chat_server --port 12345 --max-clients 50.

## scripts/spawn_clients.sh

Cria N clientes (usando nc ou examples/client_simple) para testes de estresse.

## scripts/make_release.sh

Cria tags v1-logging, v2-cli, v3-final, gera release notes básicos (usa git tag e gh release create se gh instalado).

## Exemplo comandos para tag:

```bash
git tag -a v1-logging -m "v1: libtslog + arquitetura" && git push origin v1-logging
git tag -a v2-cli -m "v2: protótipo CLI e comunicação" && git push origin v2-cli
git tag -a v3-final -m "v3: versão final" && git push origin v3-final
```

# Testes e simulação (etapa 2)

    - tests/stress_test.py: cria X conexões simultâneas, cada cliente envia Y mensagens, mede latência e detecta mensagens perdidas.

    - Unit tests para ThreadSafeQueue, SocketWrapper (simular sockets com pares de sockets), e para funções utilitárias.

# Checklist por etapas (mapear para entregas)

## Etapa 1 — v1-logging

    - Implementar tslogger wrapper e demo CLI que cria várias threads escrevendo no log.
    - Incluir docs/arquitetura.md + diagramas.
    - Tag: v1-logging.

## Etapa 2 — v2-cli

    - Implementar Server básico + client_simple (envia uma mensagem e servidor retransmite).
    - Integrar logging em eventos de conexão/mensagem.
    - Scripts de teste (spawn_clients).
    - Tag: v2-cli.

## Etapa 3 — v3-final

    - Completar broadcast robusto (Dispatcher), proteção completa de estruturas compartilhadas, limite de conexões com semáforo, monitor CLI do servidor (comandos: stats, list_clients, shutdown).
    - Relatório final com diagrama de sequência e análise IA (prompts + respostas, mapeamento de riscos de concorrência e mitigação).
    - Tag: v3-final.

# Boas práticas e observações finais

    - Mantenha cada classe pequena e com responsabilidade única (Server, ClientHandler, Logger, ThreadSafeQueue).
    - Priorize simplicidade: para nota, é melhor implementar corretamente o obrigatório (broadcast, logging, proteção) do que muitos opcionais mal feitos.
    - Documente decisões de concorrência no docs/arquitetura.md: por que usar dispatcher vs broadcast direto; quais mutexes existem; onde deadlocks foram evitados.
    - Use ferramentas de análise: valgrind --tool=helgrind / drd ou sanitizers (ThreadSanitizer) para detectar race conditions enquanto desenvolve.