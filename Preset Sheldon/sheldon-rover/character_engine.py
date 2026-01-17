import time
import random
from lines_sheldon import LINES

class CharacterEngine:
    def __init__(self):
        self.irritation = 0    # 0 upwards
        self.intensity = 0     # 0=LOW, 1=MID, 2=HIGH
        self.last_spoke = 0.0
        self.min_gap_s = 2.0
        self.mode = "sheldon"  # For future extensibility

    def on_event(self, event: str) -> str | None:
        """
        Processes an event and returns a spoken line or None.
        """
        now = time.time()
        
        # 1. Cooldown check
        if now - self.last_spoke < self.min_gap_s:
            return None

        # 2. Validate event
        if event not in LINES:
            return None

        # 3. Update state based on event
        if event in ["COLLISION", "STUCK"]:
            self.irritation += 1
        elif event == "RESET":
            self.irritation = 0
            # Reset logic: maybe return immediately? 
            # The prompt implies RESET returns a line, so we continue to selection.
        
        # 4. Determine Tier
        # Base tier is intensity.
        # Irritation >= 3 pushes tier up by 1 (capped at 2).
        tier = self.intensity
        if self.irritation >= 3:
            tier = min(2, tier + 1)
        
        # 5. Select Line
        # LINES[event] is a dict {0: [...], 1: [...], 2: [...]}
        # We need to ensure the tier exists, fallback to 0 if major error, but lines_sheldon is strict.
        possible_lines = LINES[event].get(tier, LINES[event][0])
        
        line = random.choice(possible_lines)
        
        self.last_spoke = now
        return line

    def set_intensity(self, level: int):
        """Sets the base intensity/pedantry level (0-2)."""
        self.intensity = max(0, min(2, level))
