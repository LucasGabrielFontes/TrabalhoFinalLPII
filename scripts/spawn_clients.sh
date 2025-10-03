#!/usr/bin/env bash
set -euo pipefail

NUM_CLIENTS=${1:-5}
SERVER_IP=${2:-127.0.0.1}
PORT=${3:-12345}

for i in $(seq 1 "$NUM_CLIENTS"); do
    echo "Iniciando cliente $i..."
    ./build/bin/client_simple "$SERVER_IP" "$PORT" &
done

wait