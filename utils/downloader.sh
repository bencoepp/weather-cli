#!/bin/bash

BASE_URL="https://www.ncei.noaa.gov/data/global-hourly/access/"
TARGET_DIR="../data"

# Initialize a global counter
count=1

mkdir -p "$TARGET_DIR"

download_csv_files() {
    local url="$1"
    local local_target_dir="$2"

    # If no local target directory is provided, default to the main TARGET_DIR
    [ -z "$local_target_dir" ] && local_target_dir="$TARGET_DIR"

    # Get a list of subdirectories at this level. NOAA directories often represent years.
    # We'll replicate this structure locally.
    local directories=($(curl -s "$url" | grep -oP '(?<=href=")[^"]+/' | sort -u))

    for dir in "${directories[@]}"; do
        # Remove the trailing slash from the directory name
        dir_name="${dir%/}"

        # Create a corresponding subdirectory under the local target directory
        mkdir -p "$local_target_dir/$dir_name"

        # Recursively download files from this subdirectory
        download_csv_files "${url}${dir}" "$local_target_dir/$dir_name"
    done

    # Download CSV files at the current level
    curl -s "$url" | grep -oP '(?<=href=")[^"]+\.csv' | while read -r file; do
        echo "Downloading: ${url}${file}"

        # Extract the filename from the URL
        filename="${file##*/}"

        # Download the file into the appropriate directory, prefixing with a global counter
        curl -s "${url}${file}" -o "${local_target_dir}/${count}_${filename}"

        # Increment the counter after each file
        ((count++))
    done
}

download_csv_files "$BASE_URL"
echo "Download completed. The files have been organized by directory (e.g. year) in $TARGET_DIR."