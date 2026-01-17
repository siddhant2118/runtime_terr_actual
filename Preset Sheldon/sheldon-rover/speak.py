import os
import shutil
import csv
import subprocess

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
ASSETS_DIR = os.path.join(BASE_DIR, 'assets')
DATABASE_FILE = os.path.join(ASSETS_DIR, 'database.csv')
AUDIO_DIR = os.path.join(ASSETS_DIR, 'audio')

# Cache for line -> filename
AUDIO_CACHE = {}
DB_LOADED = False

def load_audio_db():
    global DB_LOADED
    if DB_LOADED:
        return

    if not os.path.exists(DATABASE_FILE):
        return

    try:
        with open(DATABASE_FILE, 'r', newline='') as f:
            reader = csv.DictReader(f)
            for row in reader:
                if row['file_exists'] == "TRUE":
                    # Map transcript to filename
                    AUDIO_CACHE[row['transcript']] = row['audio_filename']
        DB_LOADED = True
    except Exception as e:
        print(f"⚠️ Error loading audio database: {e}")

def play_audio_file(filename):
    filepath = os.path.join(AUDIO_DIR, filename)
    if os.path.exists(filepath):
        try:
            # afplay is standard on macOS
            subprocess.run(["afplay", filepath], check=False)
            return True
        except Exception as e:
            print(f"⚠️ Error playing audio: {e}")
    return False


def speak(line: str):
    """
    Speaks the line using pre-recorded audio if available, 
    otherwise falls back to macOS 'say' command.
    Always prints the line to stdout.
    """
    print(f"🔊 {line}")
    
    # Try to play recorded audio first
    load_audio_db()
    if line in AUDIO_CACHE:
        filename = AUDIO_CACHE[line]
        print(f"🎵 Playing: {filename}")
        if play_audio_file(filename):
            return

    # Check if 'say' command exists
    if shutil.which("say"):
        try:
            # Escape double quotes for shell safety
            safe_line = line.replace('"', "'")
            os.system(f'say "{safe_line}"')

        except Exception as e:
            print(f"Error speaking: {e}")
    else:
        # Fallback if not on macOS or say is missing
        pass
