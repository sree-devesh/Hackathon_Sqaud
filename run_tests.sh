#!/bin/bash
# ── Hackathon Squad – Test Runner ──
# Compiles main.cpp, runs all test cases, and reports results.

set -e
cd "$(dirname "$0")"

BIN=./hackathon_squad
SRC=main.cpp

echo "╔══════════════════════════════════════════════════╗"
echo "║          HACKATHON SQUAD – Test Runner           ║"
echo "╚══════════════════════════════════════════════════╝"
echo

# ── compile ──
echo "⟩ Compiling $SRC ..."
g++ -O2 -std=c++17 -I. "$SRC" -o "$BIN"
echo "  ✓ Build successful"
echo

# ── run tests ──
PASS=0; BETTER=0; WORSE=0; TOTAL=0

printf "%-48s  %-12s  %s\n" "Test" "Verdict" "Score"
printf "%-48s  %-12s  %s\n" "----" "-------" "-----"

for input in test_suite/input_*.txt; do
    base=$(basename "$input" | sed 's/^input_//;s/\.txt$//')
    output="test_suite/output_${base}.txt"
    [ -f "$output" ] || continue

    TOTAL=$((TOTAL + 1))

    result=$("$BIN" < "$input" 2>/dev/null)
    got_sum=$(echo "$result" | head -1)
    exp_sum=$(head -1 "$output")

    if [ "$got_sum" = "$exp_sum" ]; then
        verdict="✓ MATCH"
        PASS=$((PASS + 1))
    elif [ "$got_sum" -gt "$exp_sum" ] 2>/dev/null; then
        verdict="★ BETTER"
        BETTER=$((BETTER + 1))
    else
        verdict="✗ WORSE"
        WORSE=$((WORSE + 1))
    fi

    printf "%-48s  %-12s  got=%-18s ref=%s\n" "$base" "$verdict" "$got_sum" "$exp_sum"
done

echo
echo "══════════════════════════════════════════════════"
printf "  Total: %d  |  Match: %d  |  Better: %d  |  Worse: %d\n" \
       "$TOTAL" "$PASS" "$BETTER" "$WORSE"
echo "══════════════════════════════════════════════════"

# ── cleanup ──
rm -f "$BIN"

if [ "$WORSE" -eq 0 ]; then
    echo "  ✓ All tests passed!"
    exit 0
else
    echo "  ✗ Some tests scored below reference."
    exit 1
fi
