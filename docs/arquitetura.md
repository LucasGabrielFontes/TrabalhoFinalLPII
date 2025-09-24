# Arquitetura — Etapa 1 (estado atual)

Objetivo desta etapa
- Disponibilizar um logger thread-safe via `TSLog` (wrapper C++) e uma demo CLI concorrente que o exercita.
- Resultado: concluído. Há logs consistentes em `logs/demo.log` com escrita concorrente.

Estrutura (arquivos usados nesta etapa)
- CMakeLists.txt
- Makefile
- include/tslogger.hpp
- src/tslogger.cpp
- src/main_logging_demo.cpp
- scripts/run_logging_demo.sh          (script de build/execução via CMake)
- docs/arquitetura.md                  (este arquivo)
- .gitignore                           (ignora build/ e logs/)
- logs/demo.log                        (gerado pela demo)

Componente principal: TSLog (wrapper)
- API (namespace TSLog)
  - Tipos: `enum class Level { Trace, Debug, Info, Warn, Error, Fatal }`
  - Config: `struct Config { std::string path; Level min_level; bool also_stderr; }`
  - Funções: `init(...)`, `log(Level, fmt, ...)`, `trace/debug/info/warn/error/fatal(...)`, `flush()`, `close()`
  - Util: `level_from_env()` (lê TSLOG_LEVEL), `level_name(Level)`
- Implementação (thread-safe)
  - Um mutex global serializa o acesso a:
    - `FILE* g_file` (arquivo de log) e escrita em stderr (opcional)
    - Configuração global `g_cfg`
  - `g_inited` (std::atomic<bool>) evita inicializações repetidas
  - Criação automática do diretório do log (std::filesystem)
  - Formato da linha:
    - `[YYYY-MM-DD HH:MM:SS.micros] LEVEL tid=<hash-thread> mensagem`
  - Regras:
    - Filtragem por nível (min_level)
    - Bufferização por linha; `flush()` automático para níveis >= ERROR
    - `also_stderr` duplica a saída no stderr (útil em desenvolvimento)

Demo concorrente: log_demo
- Arquivo: `src/main_logging_demo.cpp`
- Fluxo:
  1) Lê parâmetros CLI: `<threads> <linhas_por_thread>` (defaults 8 e 200)
  2) `TSLog::init` com `path=logs/demo.log` e nível de `TSLOG_LEVEL` (default INFO)
  3) Cria N threads; cada uma registra mensagens aleatórias entre DEBUG/INFO/WARN/ERROR
  4) Aguarda com `join()`, emite "Finalizando demo", `flush()` e `close()`
- Evidências:
  - `logs/demo.log` contém: linha de init, interleaving de múltiplos tids, e linha de finalização

Build e execução
- Makefile (padrão release):
  - `make`
  - `make run THREADS=8 LINES=200 TSLOG_LEVEL=DEBUG`
  - `make clean`
- CMake:
  - `mkdir -p build && cmake -S . -B build && cmake --build build -j`
  - `TSLOG_LEVEL=DEBUG ./build/log_demo 8 200`
- Script:
  - `chmod +x scripts/run_logging_demo.sh`
  - `TSLOG_LEVEL=DEBUG THREADS=8 LINES=200 ./scripts/run_logging_demo.sh`

Integração com third_party/libtslog (opcional)
- Diretório `third_party/libtslog` não é obrigatório nesta etapa.
- CMake detecta e linka automaticamente se esse diretório existir e exportar o target `tslog`.
- No Makefile, pode-se habilitar com `make USE_THIRD_PARTY_TSLOG=1` caso a lib esteja vendorizada/instalada.
- Atualmente, o wrapper `TSLog` funciona sozinho; a integração pode ser feita nas próximas etapas.

Decisões de projeto (Etapa 1)
- Serialização via `std::mutex` para garantir linhas atômicas no arquivo e stderr
- Timestamps com microssegundos; `flush` automático em erros
- Nível mínimo configurável por variável de ambiente (TSLOG_LEVEL)
- Foco em simplicidade e segurança de thread; sem rotação de arquivos ainda

Limitações conhecidas
- Sem rotação de log ou truncamento
- Logger síncrono (uma seção crítica por linha); suficiente para o escopo atual
- Dependência de `std::filesystem` (compiladores muito antigos podem exigir flags extras)

Próximas etapas
- Etapa 2 (v2-cli):
  - Servidor TCP básico + cliente CLI simples
  - Integração do `TSLog` a eventos de conexão/mensagem
- Etapa 3 (v3-final):
  - Dispatcher para broadcast robusto com `ThreadSafeQueue`
  - Limite de conexões (semáforo)
  - Monitor/CLI administrativo e testes de estresse

Checklist da entrega (Etapa 1)
- [x] Wrapper de logging thread-safe implementado (TSLog)
- [x] Demo CLI concorrente gerando logs
- [x] Build por Makefile e CMake
- [x] Documentação inicial (este arquivo)
- [x] `.gitignore` com build/ e logs/
- [ ] Tag repositório: `git tag -a v1-logging -m "v1: lib de logging thread-safe + arquitetura" && git push origin v1-logging`