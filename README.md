# Trabalho Final LPII — Etapa 2

## **Descrição do Projeto**
Este projeto implementa um **Servidor de Chat Multiusuário (TCP)** com suporte a múltiplos clientes conectados simultaneamente. Ele utiliza **threads** para gerenciar conexões de clientes e **mutexes** para proteger estruturas compartilhadas. Além disso, o projeto integra a biblioteca de logging thread-safe **`libtslog`** para registrar eventos importantes, como conexões, mensagens enviadas/recebidas e desconexões.

### **Funcionalidades**
- **Servidor TCP**:
  - Aceita múltiplos clientes simultaneamente.
  - Cada cliente é gerenciado por uma thread separada.
  - Mensagens enviadas por um cliente são retransmitidas para todos os outros clientes conectados (broadcast).
- **Cliente CLI**:
  - Conecta ao servidor e permite enviar mensagens.
  - Exibe mensagens recebidas de outros clientes.
- **Logging**:
  - Registra eventos como conexões, mensagens e desconexões no arquivo `logs/server.log`.
- **Scripts de Teste**:
  - Simula múltiplos clientes conectando ao servidor para testes de carga.

---

## **Como Rodar o Projeto**

### **1. Pré-requisitos**
Certifique-se de ter o seguinte instalado:
- **Linux** ou outro sistema compatível com POSIX.
- **Compilador C++** com suporte a C++20 (ex.: `g++`).
- **CMake** (opcional, se preferir usar `Makefile` diretamente).

### **2. Compilar o Projeto**

#### **Usando Makefile**
1. Compile o projeto no modo release:
   ```bash
   make
   ```

2. Ou compile no modo debug:
    ```bash
    make debug
    ```

#### **Usando CMake**
1. Crie o diretório de build:
    ```bash
    mkdir -p build
    ```

2. Gere os arquivos de build:
    ```bash
    cmake -S . -B build
    ```

3. Compile o projeto:
    ```bash
    cmake --build build -j
    ```

### **3. Executar o Servidor**

1. Inicie o servidor (padrão na porta 12345):
    ```bash
    ./build/bin/chat_server
    ```

    Para especificar outra porta:
    ```bash
    ./build/bin/chat_server 8080
    ```

2. Os logs do servidor serão gravados no arquivo server.log.

### **4. Executar o Cliente (em outro terminal)**

1. Execute o cliente CLI (padrão IP 127.0.0.1 e porta 12345):
    ```bash
    ./build/bin/client_simple
    ```

    Para especificar outro IP e porta:
    ```bash
    ./build/bin/client_simple 127.0.0.1 8080
    ```

2. Digite mensagens no cliente para enviá-las ao servidor. As mensagens serão retransmitidas para todos os outros clientes conectados.

### **5. Simular Múltiplos Clientes (em outro terminal)**

1. Use o script spawn_clients.sh para simular múltiplos clientes (iniciará 5 clientes conectados ao servidor no IP 127.0.0.1 e porta 12345.):
    ```bash
    ./scripts/spawn_clients.sh 5 127.0.0.1 12345
    ```

## **Estrutura do Projeto**

```bash
TrabalhoFinalLPII/
├── README.md                # Documentação do projeto
├── CMakeLists.txt           # Configuração do CMake
├── Makefile                 # Configuração do Makefile
├── docs/                    # Diagramas e documentação
│   ├── arquitetura.md       # Arquitetura do projeto
│   ├── diagramas/           # Diagramas de sequência e componentes
├── include/                 # Headers do projeto
│   ├── tslogger.hpp         # Wrapper para libtslog
├── src/                     # Código-fonte principal
│   ├── main_server.cpp      # Implementação do servidor
│   ├── tslogger.cpp         # Implementação do logger
├── examples/                # Exemplos de clientes
│   ├── client_simple.cpp    # Cliente CLI simples
├── scripts/                 # Scripts de automação
│   ├── spawn_clients.sh     # Simula múltiplos clientes
│   ├── run_logging_demo.sh  # Executa o demo de logging
├── logs/                    # Diretório para logs gerados
```

## **Logs**

Os eventos do servidor são registrados no arquivo server.log. Exemplos de eventos registrados:

- Inicialização do servidor.
- Conexões e desconexões de clientes.
- Mensagens enviadas e retransmitidas.

## **Próxima Etapa**

- Etapa 3: 
    - Implementar um dispatcher para broadcast robusto.
    - Adicionar limite de conexões simultâneas usando semáforos.
    - Criar um monitor CLI administrativo para o servidor.