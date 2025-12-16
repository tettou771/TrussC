#!/bin/bash
# =============================================================================
# build_all.sh - 全サンプルのビルドスクリプト
# =============================================================================
# 使い方:
#   ./build_all.sh          # 全サンプルをビルド
#   ./build_all.sh --clean  # クリーンビルド（CMakeキャッシュを削除）
#   ./build_all.sh --help   # ヘルプを表示

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CLEAN_BUILD=false
VERBOSE=false

# 色付き出力
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ヘルプ表示
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --clean    クリーンビルド（CMakeキャッシュを削除してから再ビルド）"
    echo "  --verbose  詳細なビルド出力を表示"
    echo "  --help     このヘルプを表示"
    echo ""
    echo "Examples:"
    echo "  $0              # 全サンプルをビルド"
    echo "  $0 --clean      # クリーンビルド"
}

# 引数パース
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

echo -e "${BLUE}=== TrussC Examples Build Script ===${NC}"
echo ""

# 並列ビルド用のジョブ数（CPUコア数）
JOBS=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
echo -e "Using ${JOBS} parallel jobs"
echo ""

# ビルドディレクトリを検索
BUILD_DIRS=$(find "$SCRIPT_DIR" -type d -name "build" | sort)

if [ -z "$BUILD_DIRS" ]; then
    echo -e "${RED}No build directories found!${NC}"
    exit 1
fi

# 統計用変数
TOTAL=0
SUCCESS=0
FAILED=0
FAILED_LIST=()

# 各ビルドディレクトリでビルド実行
for BUILD_DIR in $BUILD_DIRS; do
    # サンプル名を抽出（例: 3d/ofNodeExample）
    EXAMPLE_NAME=$(echo "$BUILD_DIR" | sed "s|$SCRIPT_DIR/||" | sed 's|/build$||')
    TOTAL=$((TOTAL + 1))

    echo -e "${YELLOW}[$TOTAL] Building: $EXAMPLE_NAME${NC}"

    cd "$BUILD_DIR"

    # クリーンビルドの場合、CMakeキャッシュを削除
    if [ "$CLEAN_BUILD" = true ]; then
        rm -f CMakeCache.txt
        rm -rf CMakeFiles
    fi

    # CMake configure
    if [ "$VERBOSE" = true ]; then
        if ! cmake ..; then
            echo -e "${RED}  CMake configure failed!${NC}"
            FAILED=$((FAILED + 1))
            FAILED_LIST+=("$EXAMPLE_NAME (cmake)")
            continue
        fi
    else
        if ! cmake .. > /dev/null 2>&1; then
            echo -e "${RED}  CMake configure failed!${NC}"
            FAILED=$((FAILED + 1))
            FAILED_LIST+=("$EXAMPLE_NAME (cmake)")
            continue
        fi
    fi

    # ビルド（並列ビルド）
    if [ "$VERBOSE" = true ]; then
        if cmake --build . -j "$JOBS"; then
            echo -e "${GREEN}  Success!${NC}"
            SUCCESS=$((SUCCESS + 1))
        else
            echo -e "${RED}  Build failed!${NC}"
            FAILED=$((FAILED + 1))
            FAILED_LIST+=("$EXAMPLE_NAME (build)")
        fi
    else
        if cmake --build . -j "$JOBS" > /dev/null 2>&1; then
            echo -e "${GREEN}  Success!${NC}"
            SUCCESS=$((SUCCESS + 1))
        else
            echo -e "${RED}  Build failed!${NC}"
            FAILED=$((FAILED + 1))
            FAILED_LIST+=("$EXAMPLE_NAME (build)")
        fi
    fi
done

# 結果サマリー
echo ""
echo -e "${BLUE}=== Build Summary ===${NC}"
echo -e "Total:   $TOTAL"
echo -e "Success: ${GREEN}$SUCCESS${NC}"
echo -e "Failed:  ${RED}$FAILED${NC}"

if [ $FAILED -gt 0 ]; then
    echo ""
    echo -e "${RED}Failed examples:${NC}"
    for FAILED_EXAMPLE in "${FAILED_LIST[@]}"; do
        echo -e "  - $FAILED_EXAMPLE"
    done
    exit 1
fi

echo ""
echo -e "${GREEN}All examples built successfully!${NC}"
exit 0
