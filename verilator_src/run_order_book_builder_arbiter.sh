#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OBJ_DIR="${ROOT_DIR}/verilator_src/obj_dir"

cd "${ROOT_DIR}"

verilator \
  --binary \
  --timing \
  --top-module tb_order_book_parser_builder_arbiter \
  --Mdir "${OBJ_DIR}" \
  -I"${ROOT_DIR}/market_data" \
  -I"${ROOT_DIR}/verilator_src" \
  -Wno-fatal \
  -Wno-WIDTHTRUNC \
  -Wno-UNSIGNED \
  -Wno-UNOPTFLAT \
  -F verilator_src/order_book_builder_arbiter.f

"${OBJ_DIR}/Vtb_order_book_parser_builder_arbiter"
