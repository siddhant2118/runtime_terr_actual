import os
import shutil
import json

DATA_DIR = "esp32-server/data"
MAP_FILE = os.path.join(DATA_DIR, "audio_map.json")

def shorten_filenames():
    new_map = {}
    
    # 1. Walk and rename
    for root, dirs, files in os.walk(DATA_DIR):
        if root == DATA_DIR: continue # Skip root data dir
        
        category = os.path.basename(root)
        new_map[category] = []
        
        for file in files:
            if not file.endswith(".mp3"): continue
            
            src_path = os.path.join(root, file)
            
            # Create short name
            # Strategy: cat_index.mp3 (e.g., collision_1.mp3)
            # This is safest.
            
            # But we want to preserve some meaning if possible for debugging?
            # actually, simpler is better for limited FS.
            # Let's use: category_index.mp3
            
            idx = len(new_map[category]) + 1
            ext = ".mp3"
            
            # Short name: cat_1.mp3
            # Ensure safe length (30 chars max total path? no, filename max)
            # category usually < 10 chars.
            # "saw_human_1.mp3" is safe.
            
            new_name = f"{category.lower()}_{idx}{ext}"
            dest_path = os.path.join(root, new_name)
            
            # If src != dest (it will be different), rename
            if src_path != dest_path:
                print(f"Renaming {file} -> {new_name}")
                os.rename(src_path, dest_path)
            
            new_map[category].append(new_name)

    # 2. Write new map
    with open(MAP_FILE, 'w') as f:
        json.dump(new_map, f, indent=2)
    
    print("Files Renamed & Map Updated.")

if __name__ == "__main__":
    shorten_filenames()
