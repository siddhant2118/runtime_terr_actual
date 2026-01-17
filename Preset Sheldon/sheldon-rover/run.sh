#!/bin/bash
set -e
cd "$(dirname "$0")"

# Create venv if it doesn't exist
if [ ! -d ".venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv .venv
fi

source .venv/bin/activate

# Install requirements if file exists (currently empty/optional as per prompt, but good practice)
if [ -f "requirements.txt" ]; then
    pip install -r requirements.txt > /dev/null 2>&1 || true
fi

python demo_layerA.py
