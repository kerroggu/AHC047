#!/bin/bash

g++ a.cpp -o a.out -std=c++17 -O2 -Wall

# --- 設定 (以前のスクリプトと同様) ---
cd "$(dirname "$0")"
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
if [ ! -f "$TESTER_EXECUTABLE" ]; then
    echo "Error: $TESTER_EXECUTABLE not found." >&2
    exit 1
fi
mkdir -p "$OUTPUT_DIR"
> "$SCORE_FILE" # スコアファイルを初期化

# --- 並列実行数を設定 ---
NPROC=$(nproc 2>/dev/null || echo 4)
MAX_JOBS=$NPROC
MAX_JOBS=4

echo "Starting tests with up to $MAX_JOBS parallel jobs..."

# --- テストケース番号のリストを生成 ---
find "$INPUT_DIR" -maxdepth 1 -name '????.txt' -printf '%f\n' | sed 's/\.txt$//' | sort | \
xargs -I {} -P "$MAX_JOBS" \
bash -c '
    filenum="{}"  # xargsから渡されたファイル番号
    input_file="'"$INPUT_DIR"'/$filenum.txt"
    output_file="'"$OUTPUT_DIR"'/out$filenum.txt"
    err_file="'"$ERR_DIR"'/err$filenum.txt"  # エラーログファイルを設定
    cpp_exec="'"$CPP_EXECUTABLE"'"
    tester_exec="'"$TESTER_EXECUTABLE"'"
    score_file="'"$SCORE_FILE"'"

    echo "Starting test case $filenum..."

    # コマンド実行 (標準エラーを err_file にリダイレクト)
    #cat "$input_file" | "$tester_exec" wsl "$cpp_exec" > "$output_file" 2> "$err_file"
    "$cpp_exec" < "$input_file"  > "$output_file" 2> "$err_file"

    status=$?  # wsl コマンドの終了ステータス

    # スコアファイルに結果を追記
    if [ $status -ne 0 ]; then
        echo "Case $filenum: Tester Failed (status: $status)" >> "$score_file"
        echo "Test case $filenum Failed (tester exited with status: $status)"
    elif grep -q "^Best" "$err_file"; then
        grep "^Best" "$err_file" | sed "s/^/Case $filenum: /" >> "$score_file"
        echo "Test case $filenum Done."
    else
        echo "Case $filenum: Failed (No Score line found)" >> "$score_file"
        echo "Test case $filenum Failed (No Score line found)"
    fi

    # 出力ファイルが空でないか確認
    if [ $status -eq 0 ] && [ ! -s "$output_file" ]; then
        echo "Warning: Output file '"$output_file"' is empty for test case $filenum." >&2
    fi
'

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
