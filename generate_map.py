import os
import json

DATA_DIR = "esp32-server/data"
OUTPUT_FILE = "esp32-server/data/audio_map.json"

def generate_map():
    audio_map = {}
    
    for root, dirs, files in os.walk(DATA_DIR):
        category = os.path.basename(root)
        if root == DATA_DIR: continue # Skip root
        
        audio_files = [f for f in files if f.endswith('.mp3')]
        if audio_files:
            audio_map[category] = audio_files

    with open(OUTPUT_FILE, 'w') as f:
        json.dump(audio_map, f, indent=2)
    
    print(f"Generated {OUTPUT_FILE}")
    print(json.dumps(audio_map, indent=2))

if __name__ == "__main__":
    generate_map()
