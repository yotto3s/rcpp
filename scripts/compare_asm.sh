#!/usr/bin/env bash
# compare_asm.sh — Compare assembly of refined_* vs plain_* function pairs
#
# Usage: compare_asm.sh <build-examples-dir> <prefix> <example-names...>
# Example: compare_asm.sh build/examples zero_overhead_ 01_value_passthrough 02_arithmetic

set -euo pipefail

BUILD_DIR="$1"
PREFIX="$2"
shift 2
EXAMPLES=("$@")

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
RESET='\033[0m'

PASS=0
DIFF=0
TOTAL=0

DUMPFILE=$(mktemp)
trap 'rm -f "$DUMPFILE"' EXIT

# Extract a function body from objdump output file by matching the full header line.
# The header has the form: <hex-addr> <full_demangled_name(...)>:
# We grep for lines matching the short name prefix, then extract between
# that header and the next blank line.
extract_func_body() {
    local dumpfile="$1"
    local short_name="$2"

    # Find the header line number for this function (exclude cold clones)
    local header
    header=$(grep -n "^[0-9a-f]* <${short_name}(" "$dumpfile" | grep -v '\[clone \.cold\]' | head -1 | cut -d: -f1)

    if [[ -z "$header" ]]; then
        return 1
    fi

    # From the header+1, print until blank line; strip address prefixes; normalize.
    # The trailing `|| true` prevents pipefail from propagating grep -v exit code 1
    # when all lines are filtered out (e.g., only NOPs remain after the ret).
    tail -n +"$((header + 1))" "$dumpfile" \
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
        | sed 's/[[:space:]]*$//' \
        || true
}

for example in "${EXAMPLES[@]}"; do
    binary="${BUILD_DIR}/${PREFIX}${example}"

    if [[ ! -f "$binary" ]]; then
        echo -e "${RED}SKIP${RESET} ${example}: binary not found at ${binary}"
        continue
    fi

    # Get full disassembly into a temp file (avoids NUL-byte issues with bash variables)
    objdump -d --no-show-raw-insn "$binary" | c++filt > "$DUMPFILE"

    # Find all refined_* short function names from header lines
    REFINED_FUNCS=$(grep -oP '(?<=<)refined_[a-zA-Z0-9_]+(?=\()' "$DUMPFILE" \
        | sort -u || true)

    if [[ -z "$REFINED_FUNCS" ]]; then
        echo -e "${YELLOW}SKIP${RESET} ${example}: no refined_* functions found"
        continue
    fi

    for refined_func in $REFINED_FUNCS; do
        plain_func="${refined_func/refined_/plain_}"
        TOTAL=$((TOTAL + 1))

        refined_asm=$(extract_func_body "$DUMPFILE" "$refined_func" 2>/dev/null) || refined_asm=""
        plain_asm=$(extract_func_body "$DUMPFILE" "$plain_func" 2>/dev/null) || plain_asm=""

        # Normalize commutative lea operand order (register allocation artifact)
        refined_asm=$(echo "$refined_asm" | perl -pe 's/lea\s+\((%\w+),(%\w+)(?:,1)?\)/my @r = sort ($1,$2); "lea ($r[0],$r[1],1)"/e')
        plain_asm=$(echo "$plain_asm" | perl -pe 's/lea\s+\((%\w+),(%\w+)(?:,1)?\)/my @r = sort ($1,$2); "lea ($r[0],$r[1],1)"/e')

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
