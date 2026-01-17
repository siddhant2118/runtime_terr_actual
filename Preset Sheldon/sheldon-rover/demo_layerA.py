import sys
from character_engine import CharacterEngine
import speak

def main():
    engine = CharacterEngine()
    
    print("🤖 Sheldon-Energy Rover Engine (Layer A)")
    print("----------------------------------------")
    print("Commands:")
    print("  [EVENT_NAME]   e.g. BOOT, COLLISION, RESET, MOVE_START, STUCK")
    print("  intensity [0-2]  Set base strictness level")
    print("  h                Human Detected (SAW_HUMAN)")
    print("  h                Human Detected (SAW_HUMAN)")
    print("  r                Random Line (RANDOM)")
    print("  m                Malarkey (COLLISION High)")
    print("  o                Overwhelmed (STUCK High)")
    print("  quit             Exit")
    print("----------------------------------------")

    # Initial boot line
    if line := engine.on_event("BOOT"):
        speak.speak(line)

    while True:
        try:
            user_input = input("> ").strip().upper()
        except EOFError:
            break

        if not user_input:
            continue

        if user_input == "QUIT":
            break
            
        if user_input.startswith("INTENSITY"):
            parts = user_input.split()
            if len(parts) == 2 and parts[1].isdigit():
                val = int(parts[1])
                engine.set_intensity(val)
                print(f"ℹ️ Intensity set to {engine.intensity}")
                if val == 2:
                    speak.speak("Activating strict correctness mode.")
            else:
                print("Usage: intensity [0|1|2]")
            continue

        # Mappings
        if user_input == "H":
            user_input = "SAW_HUMAN"
        elif user_input == "R":
            user_input = "RANDOM"
        elif user_input == "M":
             # Force high intensity collision
             engine.set_intensity(2)
             user_input = "COLLISION"
        elif user_input == "O":
             # Force high intensity stuck
             engine.set_intensity(2)
             user_input = "STUCK"


        # Treat as event
        line = engine.on_event(user_input)
        if line:
            speak.speak(line)
        else:
            # Could be invalid event or cooldown
            # For demo purposes, if it's invalid event, maybe warn?
            # But the prompt said "Type random garbage -> should not crash; may return None"
            # So we just do nothing or print nothing.
            pass

if __name__ == "__main__":
    main()
