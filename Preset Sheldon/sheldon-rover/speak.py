import os
import shutil

def speak(line: str):
    """
    Speaks the line using macOS 'say' command if available.
    Always prints the line to stdout.
    """
    print(f"🔊 {line}")
    
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
