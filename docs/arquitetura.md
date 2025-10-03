# Arquitetura — Etapa 2 (Protótipo CLI de Comunicação)

## **Objetivo**
Implementar um protótipo funcional de cliente/servidor em rede, com suporte a múltiplos clientes conectados simultaneamente. O servidor retransmite mensagens enviadas por um cliente para todos os outros (broadcast). O projeto utiliza logging thread-safe para registrar eventos importantes.

---

## **Estrutura do Projeto**
- **Servidor**:
  - Aceita conexões TCP de múltiplos clientes.
  - Cada cliente é gerenciado por uma thread separada.
  - Mensagens são retransmitidas para todos os clientes conectados.
- **Cliente**:
  - Conecta ao servidor e permite enviar/receber mensagens.
- **Logging**:
  - Registra eventos como conexões, mensagens e desconexões no arquivo [server.log](http://_vscodecontentref_/5).

---

## **Fluxo de Execução**
### **Servidor**
1. Inicializa o logger (`TSLog`).
2. Cria um socket TCP e escuta conexões na porta especificada.
3. Para cada cliente conectado:
   - Cria uma thread para gerenciar a conexão.
   - Recebe mensagens do cliente e retransmite para os demais.
4. Registra eventos no log.

### **Cliente**
1. Conecta ao servidor no IP e porta especificados.
2. Envia mensagens digitadas pelo usuário.
3. Exibe mensagens recebidas de outros clientes.