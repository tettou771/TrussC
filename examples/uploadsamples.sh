#!/bin/bash
# =============================================================================
# uploadsamples.sh - Screenshot capture & R2 upload for TrussC samples
# =============================================================================
# Usage:
#   ./uploadsamples.sh              # Process all samples
#   ./uploadsamples.sh --dry-run    # Test without uploading
#   ./uploadsamples.sh sample_name  # Process specific sample only
# =============================================================================

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TC_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SAMPLES_DIR="/tmp/trussc-samples"
WASM_BUCKET="trussc-wasm"
SCREENSHOT_DELAY=1.5  # Wait time after launch (seconds)

# Colored output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[OK]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Prepare output directories
mkdir -p "$SAMPLES_DIR"/{wasm,thumbs}

# Collect sample directories
collect_samples() {
    local samples=()

    # examples/category/sample
    for dir in "$TC_ROOT"/examples/*/*; do
        if [ -d "$dir" ] && [ -f "$dir/CMakeLists.txt" ]; then
            samples+=("$dir")
        fi
    done

    # addons/addon_name/example-*
    for dir in "$TC_ROOT"/addons/*/example-*; do
        if [ -d "$dir" ] && [ -f "$dir/CMakeLists.txt" ]; then
            samples+=("$dir")
        fi
    done

    printf '%s\n' "${samples[@]}"
}

# Get description from README.md
get_description() {
    local dir="$1"
    local readme="$dir/README.md"

    if [ -f "$readme" ]; then
        # Get first non-empty, non-header line
        head -n 5 "$readme" | grep -v '^#' | grep -v '^$' | head -n 1 | sed 's/^[[:space:]]*//'
    else
        echo ""
    fi
}

# Determine category from directory structure
get_category() {
    local dir="$1"
    local parent="$(basename "$(dirname "$dir")")"

    case "$parent" in
        3d|graphics|sound|video|input_output|communication|utils|threads|windowing|tools)
            echo "$parent"
            ;;
        *)
            echo ""
            ;;
    esac
}

# Get addon name if sample is from an addon
get_addon() {
    local dir="$1"

    if [[ "$dir" == *"/addons/"* ]]; then
        local addon_dir="$(dirname "$dir")"
        basename "$addon_dir"
    else
        echo ""
    fi
}

# Take screenshot of running sample
take_screenshot() {
    local name="$1"
    local bin_path="$2"
    local output="$3"

    if [ ! -x "$bin_path" ]; then
        log_warn "Binary not found: $bin_path"
        return 1
    fi

    log_info "Starting $name for screenshot..."

    # Launch in background
    "$bin_path" &
    local pid=$!

    # Wait for app to initialize
    sleep "$SCREENSHOT_DELAY"

    # Capture screenshot
    if tcdebug screenshot "$output" 2>/dev/null; then
        log_success "Screenshot saved: $output"
    else
        log_warn "Failed to take screenshot for $name"
    fi

    # Terminate process
    kill "$pid" 2>/dev/null || true
    wait "$pid" 2>/dev/null || true
}

# Generate thumbnail from screenshot
generate_thumbnail() {
    local input="$1"
    local output="$2"

    if [ ! -f "$input" ]; then
        log_warn "Screenshot not found: $input"
        return 1
    fi

    # Resize to max 320px on longest side using ImageMagick
    if command -v convert &>/dev/null; then
        convert "$input" -resize 320x320\> "$output"
        log_success "Thumbnail generated: $output"
    elif command -v magick &>/dev/null; then
        magick "$input" -resize 320x320\> "$output"
        log_success "Thumbnail generated: $output"
    else
        log_warn "ImageMagick not found, copying original"
        cp "$input" "$output"
    fi
}

# Copy WASM build artifacts
copy_wasm_files() {
    local name="$1"
    local build_dir="$2"

    local html_file="$build_dir/bin/$name.html"
    local js_file="$build_dir/bin/$name.js"
    local wasm_file="$build_dir/bin/$name.wasm"

    if [ -f "$html_file" ] && [ -f "$js_file" ] && [ -f "$wasm_file" ]; then
        cp "$html_file" "$SAMPLES_DIR/wasm/"
        cp "$js_file" "$SAMPLES_DIR/wasm/"
        cp "$wasm_file" "$SAMPLES_DIR/wasm/"
        log_success "WASM files copied: $name"
        return 0
    else
        log_warn "WASM files not found for: $name"
        return 1
    fi
}

# Generate samples.json metadata file
generate_samples_json() {
    log_info "Generating samples.json..."

    local examples_json="[]"
    local addons_json="[]"

    for sample_dir in $(collect_samples); do
        local name="$(basename "$sample_dir")"
        local title="$(echo "$name" | sed 's/Example$//' | sed 's/\([a-z]\)\([A-Z]\)/\1 \2/g')"
        local desc="$(get_description "$sample_dir")"
        local category="$(get_category "$sample_dir")"
        local addon="$(get_addon "$sample_dir")"

        # Skip if WASM files don't exist
        if [ ! -f "$SAMPLES_DIR/wasm/$name.html" ]; then
            continue
        fi

        local json_obj=$(cat <<EOF
{
    "name": "$name",
    "title": "$title",
    "description": "$desc",
    "thumb": "thumbs/$name.png",
    "wasm": "wasm/$name.html"
EOF
)

        if [ -n "$addon" ]; then
            json_obj+=",\"addon\": \"$addon\""
            json_obj+="}"
            addons_json=$(echo "$addons_json" | jq --argjson obj "$json_obj" '. += [$obj]')
        else
            if [ -n "$category" ]; then
                json_obj+=",\"category\": \"$category\""
            fi
            json_obj+="}"
            examples_json=$(echo "$examples_json" | jq --argjson obj "$json_obj" '. += [$obj]')
        fi
    done

    # Output combined JSON
    jq -n \
        --argjson examples "$examples_json" \
        --argjson addons "$addons_json" \
        '{examples: $examples, addons: $addons}' \
        > "$SAMPLES_DIR/samples.json"

    log_success "samples.json generated"
}

# Upload to Cloudflare R2
upload_to_r2() {
    if [ "$DRY_RUN" = true ]; then
        log_info "[DRY-RUN] Would upload to R2:"
        ls -la "$SAMPLES_DIR"
        return
    fi

    log_info "Uploading to Cloudflare R2..."

    # Upload WASM files
    for file in "$SAMPLES_DIR"/wasm/*; do
        local filename=$(basename "$file")
        wrangler r2 object put "$WASM_BUCKET/wasm/$filename" --file "$file"
    done

    # Upload thumbnails
    for file in "$SAMPLES_DIR"/thumbs/*; do
        local filename=$(basename "$file")
        wrangler r2 object put "$WASM_BUCKET/thumbs/$filename" --file "$file"
    done

    # Upload samples.json
    wrangler r2 object put "$WASM_BUCKET/samples.json" --file "$SAMPLES_DIR/samples.json"

    log_success "Upload complete!"
}

# Main entry point
main() {
    local target_sample=""
    DRY_RUN=false

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            *)
                target_sample="$1"
                shift
                ;;
        esac
    done

    log_info "TrussC Sample Uploader"
    log_info "TC_ROOT: $TC_ROOT"
    log_info "Output: $SAMPLES_DIR"

    # Process samples
    for sample_dir in $(collect_samples); do
        local name="$(basename "$sample_dir")"

        # Skip if specific sample is targeted
        if [ -n "$target_sample" ] && [ "$name" != "$target_sample" ]; then
            continue
        fi

        log_info "Processing: $name"

        # Native binary path
        local bin_path="$sample_dir/bin/$name"
        if [ "$(uname)" = "Darwin" ]; then
            bin_path="$sample_dir/bin/${name}.app/Contents/MacOS/$name"
        fi

        # Take screenshot
        local shot_path="/tmp/${name}_shot.png"
        if take_screenshot "$name" "$bin_path" "$shot_path"; then
            # Generate thumbnail
            generate_thumbnail "$shot_path" "$SAMPLES_DIR/thumbs/$name.png"
        fi

        # Copy WASM files
        copy_wasm_files "$name" "$sample_dir/build-web"
    done

    # Generate samples.json
    generate_samples_json

    # Upload to R2
    upload_to_r2

    log_success "All done!"
}

main "$@"
