#!/bin/bash

# Always work relative to the location of this script
cd "$(dirname "$0")"

# Build the solver
g++ edge_30.cpp -o a.out -std=c++20 -O2 -Wall

# --- 設定 (以前のスクリプトと同様) ---
CPP_EXECUTABLE="./a.out"
TESTER_EXECUTABLE="./tester.exe"
INPUT_DIR="./in"
OUTPUT_DIR="./out"
ERR_DIR="./err"
SCORE_FILE="score.txt"

# --- 実行前準備 (以前のスクリプトと同様) ---
if [ ! -f "$CPP_EXECUTABLE" ]; then
    echo "Error: $CPP_EXECUTABLE not found." >&2
    exit 1
fi
mkdir -p "$OUTPUT_DIR"
mkdir -p "$ERR_DIR"
> "$SCORE_FILE" # スコアファイルを初期化

# --- 並列実行数を設定 ---
NPROC=$(nproc 2>/dev/null || echo 4)
MAX_JOBS=$NPROC
#MAX_JOBS=1

echo "Starting tests with up to $MAX_JOBS parallel jobs..."

# --- テストケース番号のリストを生成 ---
find "$INPUT_DIR" -maxdepth 1 -name '????.txt' -printf '%f\n' | sed 's/\.txt$//' | sort | head -n 15 | \
xargs -P "$MAX_JOBS" -I{} \
bash -c '
    filenum="$1"  # xargsから渡されたファイル番号
    input_file="'$INPUT_DIR'/$filenum.txt"
    output_file="'$OUTPUT_DIR'/out_$filenum.txt"
    err_file="'$ERR_DIR'/err_$filenum.txt"  # エラーログファイルを設定
    cpp_exec="'$CPP_EXECUTABLE'"
    tester_exec="'$TESTER_EXECUTABLE'"
    score_file="'$SCORE_FILE'"

    echo "Starting test case $filenum..."

    # コマンド実行 (標準エラーを err_file にリダイレクト)
    #cat "$input_file" | "$tester_exec" wsl "$cpp_exec" > "$output_file" 2> "$err_file"
    "$cpp_exec" < "$input_file"  > "$output_file" 2> "$err_file"

    status=$?  # wsl コマンドの終了ステータス

    # Compute score using Python scorer
    if [ $status -ne 0 ]; then
        echo "Case $filenum: Solver Failed (status: $status)" >> "$score_file"
        echo "Test case $filenum Failed (solver exited with status: $status)"
    else
        score=$(python3 compute_score.py "$input_file" "$output_file" 2>> "$err_file")
        if [ $? -ne 0 ]; then
            echo "Case $filenum: Scoring Failed" >> "$score_file"
            echo "Test case $filenum Scoring Failed"
        else
            echo "Case $filenum: Score = $score" >> "$score_file"
            echo "Test case $filenum Done."
        fi
    fi

    # 出力ファイルが空でないか確認
    if [ $status -eq 0 ] && [ ! -s "$output_file" ]; then
        echo "Warning: Output file '"$output_file"' is empty for test case $filenum." >&2
    fi
' _ {}

echo "All test cases finished processing. Scores saved in $SCORE_FILE"

# --- 合計スコア計算 ---
total_score=0
score_count=0
while IFS= read -r line; do
    if [[ "$line" =~ Score\ =\ ([0-9]+) ]]; then
        score="${BASH_REMATCH[1]}"
        total_score=$((total_score + score))
        score_count=$((score_count + 1))
    fi
done < "$SCORE_FILE"
if [ $score_count -gt 0 ]; then
    echo "-------------------------------------"
    echo "Total Score ($score_count cases): $total_score"
    echo "-------------------------------------"
fi
