#!/usr/bin/env sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT_DIR="${TMPDIR:-/tmp}/cat-chess-ai-tuning"
BIN="$OUT_DIR/cat_chess_ai_tuning"

mkdir -p "$OUT_DIR"

cc \
  -std=c99 \
  -O2 \
  -DCAT_CHESS_AI_TUNING=1 \
  -DCAT_CHESS_AI_TUNING_LOG=1 \
  -DCAT_CHESS_AI_TUNING_MAIN=1 \
  -I"$ROOT_DIR/src" \
  "$ROOT_DIR/src/chess/chess_ai.c" \
  "$ROOT_DIR/src/chess/chess_ai_tuning.c" \
  "$ROOT_DIR/src/chess/chess_board.c" \
  "$ROOT_DIR/src/chess/chess_fen.c" \
  "$ROOT_DIR/src/chess/chess_move.c" \
  "$ROOT_DIR/src/chess/chess_rules.c" \
  -o "$BIN"

"$BIN"
