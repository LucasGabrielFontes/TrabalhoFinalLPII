# CMake
cmake_minimum_required(VERSION 3.15)
project(TrabalhoFinalLPII LANGUAGES CXX)

# Configuração do padrão C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

# Opção para usar third_party/libtslog
option(USE_THIRD_PARTY_TSLOG "Usar third_party/libtslog se disponível" ON)

# Diretórios de include
include_directories(${CMAKE_SOURCE_DIR}/include)

# Adicionar subdiretório third_party/libtslog se disponível
if (USE_THIRD_PARTY_TSLOG AND EXISTS "${CMAKE_SOURCE_DIR}/third_party/libtslog/CMakeLists.txt")
    message(STATUS "Habilitando third_party/libtslog")
    add_subdirectory(third_party/libtslog)
endif()

# Biblioteca tslogger
add_library(tslogger
    src/tslogger.cpp
)
target_include_directories(tslogger PUBLIC include)
if (TARGET tslog)
    # Caso a lib tslog exporte um target "tslog"
    target_link_libraries(tslogger PRIVATE tslog)
    target_compile_definitions(tslogger PRIVATE TSLOGGER_WITH_TSLOG=1)
endif()

# Executável para o demo de logging
add_executable(log_demo
    src/main_logging_demo.cpp
)
target_link_libraries(log_demo PRIVATE tslogger)

# Executável para o servidor de chat
add_executable(chat_server
    src/main_server.cpp
)
target_link_libraries(chat_server PRIVATE tslogger)

# Executável para o cliente CLI
add_executable(client_simple
    examples/client_simple.cpp
)