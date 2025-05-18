#!/bin/bash
set -e

cd "$(dirname "$0")"

# Compile solver
if ! g++ a.cpp -o a.out -std=c++17 -O2 -Wall; then
    echo "Compilation failed" >&2
    exit 1
fi

CPP_EXECUTABLE="./a.out"
INPUT_DIR="./in"
OUTPUT_DIR="./out"
ERR_DIR="./err"
SCORE_FILE="score.txt"
SCORE_SCRIPT="./compute_score.py"

mkdir -p "$OUTPUT_DIR" "$ERR_DIR"
> "$SCORE_FILE"

NPROC=$(nproc 2>/dev/null || echo 4)
MAX_JOBS=$NPROC
MAX_JOBS=4

echo "Starting tests with up to $MAX_JOBS parallel jobs..."

find "$INPUT_DIR" -maxdepth 1 -name '????.txt' -printf '%f\n' | sort | \
    xargs -I {} -P "$MAX_JOBS" bash -e -c '
        filenum="{}"
        base="${filenum%.txt}"
        input_file="'$INPUT_DIR'/${filenum}"
        output_file="'$OUTPUT_DIR'/out_${base}.txt"
        err_file="'$ERR_DIR'/err_${base}.txt"
        score_file="'$SCORE_FILE'"
        cpp_exec="'$CPP_EXECUTABLE'"
        score_script="'$SCORE_SCRIPT'"

        echo "Running case ${base}..."
        "$cpp_exec" < "$input_file" > "$output_file" 2> "$err_file"
        if [ -s "$output_file" ]; then
            score=$(python3 "$score_script" "$input_file" "$output_file")
            echo "Case ${base}: Score = ${score}" >> "$score_file"
            echo "Case ${base} Done."
        else
            echo "Case ${base}: Failed (empty output)" >> "$score_file"
            echo "Case ${base} Failed (empty output)"
        fi
    '

echo "All test cases finished. Scores saved in $SCORE_FILE"

total_score=$(grep -oE "Score = [0-9]+" "$SCORE_FILE" | awk '{sum+=$3} END {print sum}')
case_count=$(grep -c "Score =" "$SCORE_FILE" || true)
if [ "$case_count" -gt 0 ]; then
    echo "-------------------------------------"
    echo "Total Score ($case_count cases): $total_score"
    echo "-------------------------------------"
fi
