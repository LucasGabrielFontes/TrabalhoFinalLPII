#!/usr/bin/env bash
set -euo pipefail
OUT_DIR="docs/diagramas"
mkdir -p "$OUT_DIR"

if ! command -v npx >/dev/null 2>&1; then
  echo "Instalando mermaid-cli local (npx)..."
fi

fail=0
for f in componentes seq_mensagem concorrencia log_init broadcast_direto; do
  SRC="$OUT_DIR/$f.mmd"
  [ -f "$SRC" ] || { echo "Aviso: $SRC não existe, pulando"; continue; }
  echo "Gerando $f..."
  if ! npx -y @mermaid-js/mermaid-cli -i "$SRC" -o "$OUT_DIR/$f.png"; then
     echo "ERRO ao gerar $f.png"
     fail=1
  else
     echo "OK: $OUT_DIR/$f.png"
  fi
done
exit $fail