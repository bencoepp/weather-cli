#!/usr/bin/env bash
set -euo pipefail

BASE_URL="https://www.ncei.noaa.gov/data/global-hourly/access/"
TARGET_DIR="//bcp-storage/experimental/noaa-global-hourly"

# Create the target directory if it doesn't exist
mkdir -p "$TARGET_DIR"

count=1

# Fetch the main page and extract year directories (like 1901/, 1902/, etc.)
year_dirs=$(curl -s "$BASE_URL" | grep -Eo 'href="[0-9]{4}/"' | sed 's/href="//; s/"//')

# Iterate over each year directory
for year_dir in $year_dirs; do
    year_url="${BASE_URL}${year_dir}"

    # For each year directory, fetch listing and extract .csv file links
    csv_files=$(curl -s "$year_url" | grep -Eo 'href="[^"]+\.csv"' | sed 's/href="//; s/"//')

    # Download each CSV file directly to TARGET_DIR with new names
    for csv_file in $csv_files; do
        csv_url="${year_url}${csv_file}"

        # Download directly with a sequential name
        outfile="$TARGET_DIR/file$count.csv"
        if curl --fail -s -S "$csv_url" -o "$outfile"; then
            echo "Downloaded: $csv_url -> $outfile"
            count=$((count + 1))
        else
            echo "Failed to download $csv_url" >&2
        fi
    done
done

echo "Downloaded and renamed $((count-1)) CSV files into $TARGET_DIR."