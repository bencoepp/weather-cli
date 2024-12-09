#!/bin/bash

# Define the base URL
BASE_URL="https://www.ncei.noaa.gov/data/global-hourly/access/1901/"

# Define the target directory where files will be saved
TARGET_DIR="../data"

# Create the target directory if it doesn't exist
mkdir -p "$TARGET_DIR"

# Use curl to download all .csv files
curl -s "$BASE_URL" | \
    grep -oP 'href="\K[^"]+\.csv' | \
    while read -r file; do
        curl -O "$BASE_URL$file" --output-dir "$TARGET_DIR"
    done

# Notify the user when done
echo "Download completed. All .csv files are saved in $TARGET_DIR."