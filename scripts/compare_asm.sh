#!/usr/bin/env bash
# compare_asm.sh — Compare assembly of refined_* vs plain_* function pairs
#
# Usage: compare_asm.sh <build-examples-dir> <example-names...>
# Example: compare_asm.sh build/examples 01_value_passthrough 02_arithmetic

set -euo pipefail

BUILD_DIR="$1"
shift
EXAMPLES=("$@")

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
RESET='\033[0m'

PASS=0
DIFF=0
TOTAL=0

# Extract a function body from objdump output by matching the full header line.
# The header has the form: <hex-addr> <full_demangled_name(...)>:
# We grep for lines matching the short name prefix, then extract between
# that header and the next blank line.
extract_func_body() {
    local dump="$1"
    local short_name="$2"

    # Find the header line number for this function
    local header
    header=$(echo "$dump" | grep -n "^[0-9a-f]* <${short_name}(" | head -1 | cut -d: -f1)

    if [[ -z "$header" ]]; then
        return 1
    fi

    # From the header+1, print until blank line; strip address prefixes; normalize
    echo "$dump" \
        | tail -n +"$((header + 1))" \
        | sed '/^$/q' \
        | grep -v '^$' \
        | sed 's/^[[:space:]]*[0-9a-f]*:[[:space:]]*//' \
        | grep -v '^nop' \
        | grep -v '^nopl' \
        | grep -v '^nopw' \
        | grep -v '^data16' \
        | grep -v '^xchg.*%ax,%ax' \
        | grep -v '^cs nop' \
        | sed 's/0x[0-9a-f]*(%rip)/RIPREL(%rip)/g' \
        | sed 's/#.*$//' \
        | sed 's/ <.*$//' \
        | sed 's/[0-9a-f]\{4,\}/ADDR/g' \
        | sed 's/[[:space:]]*$//'
}

for example in "${EXAMPLES[@]}"; do
    binary="${BUILD_DIR}/zero_overhead_${example}"

    if [[ ! -f "$binary" ]]; then
        echo -e "${RED}SKIP${RESET} ${example}: binary not found at ${binary}"
        continue
    fi

    # Get full disassembly once
    DUMP=$(objdump -d --no-show-raw-insn "$binary" | c++filt)

    # Find all refined_* short function names from header lines
    REFINED_FUNCS=$(echo "$DUMP" \
        | grep -oP '(?<=<)refined_[a-zA-Z0-9_]+(?=\()' \
        | sort -u || true)

    if [[ -z "$REFINED_FUNCS" ]]; then
        echo -e "${YELLOW}SKIP${RESET} ${example}: no refined_* functions found"
        continue
    fi

    for refined_func in $REFINED_FUNCS; do
        plain_func="${refined_func/refined_/plain_}"
        TOTAL=$((TOTAL + 1))

        refined_asm=$(extract_func_body "$DUMP" "$refined_func" 2>/dev/null) || refined_asm=""
        plain_asm=$(extract_func_body "$DUMP" "$plain_func" 2>/dev/null) || plain_asm=""

        if [[ -z "$refined_asm" ]]; then
            echo -e "${RED}MISS${RESET} ${example}: ${refined_func} not found in disassembly"
            DIFF=$((DIFF + 1))
            continue
        fi

        if [[ -z "$plain_asm" ]]; then
            echo -e "${RED}MISS${RESET} ${example}: ${plain_func} not found in disassembly"
            DIFF=$((DIFF + 1))
            continue
        fi

        if diff <(echo "$refined_asm") <(echo "$plain_asm") > /dev/null 2>&1; then
            echo -e "${GREEN}PASS${RESET} ${example}: ${refined_func} == ${plain_func}"
            PASS=$((PASS + 1))
        else
            echo -e "${RED}DIFF${RESET} ${example}: ${refined_func} != ${plain_func}"
            diff --color=always <(echo "$refined_asm") <(echo "$plain_asm") || true
            echo ""
            DIFF=$((DIFF + 1))
        fi
    done
done

echo ""
echo -e "${BOLD}Results: ${GREEN}${PASS} passed${RESET}, ${RED}${DIFF} differ${RESET} (${TOTAL} total)"

if [[ $DIFF -gt 0 ]]; then
    exit 1
fi
