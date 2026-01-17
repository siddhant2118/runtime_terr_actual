# audio_player.py - Audio Playback for Sheldon Rover
# Supports multiple audio backends: DFPlayer Mini, I2S DAC, or simple print (debug)

from config import DEBUG

# =============================================================================
# Audio Backend Base Class
# =============================================================================

class AudioPlayer:
    """Base audio player - just prints (for testing without hardware)."""
    
    def __init__(self):
        if DEBUG:
            print("[AUDIO] Initialized in DEBUG/print mode")
    
    def speak(self, text: str):
        """Speak the given text."""
        print(f"🔊 {text}")
    
    def stop(self):
        """Stop current playback."""
        pass


# =============================================================================
# DFPlayer Mini Backend (Recommended for loud, clear audio)
# =============================================================================

class DFPlayerAudio(AudioPlayer):
    """
    Audio player using DFPlayer Mini module.
    
    Requires:
        - DFPlayer Mini connected via UART
        - MP3 files on SD card named: 0001.mp3, 0002.mp3, etc.
        - A mapping from text hash to file number
    """
    
    def __init__(self, tx_pin: int, rx_pin: int):
        from machine import UART, Pin
        
        self.uart = UART(1, baudrate=9600, tx=Pin(tx_pin), rx=Pin(rx_pin))
        self.volume = 25  # 0-30
        
        # Initialize DFPlayer
        self._send_command(0x06, self.volume)  # Set volume
        
        if DEBUG:
            print(f"[DFPLAYER] Initialized on TX={tx_pin}, RX={rx_pin}")
    
    def _send_command(self, cmd: int, param: int = 0):
        """Send command to DFPlayer Mini."""
        # DFPlayer command format: 7E FF 06 CMD 00 PARAM_H PARAM_L EF
        buf = bytearray([
            0x7E, 0xFF, 0x06, cmd, 0x00,
            (param >> 8) & 0xFF,
            param & 0xFF,
            0xEF
        ])
        self.uart.write(buf)
    
    def speak(self, text: str):
        """Play audio file corresponding to text."""
        # Simple hash to file number mapping
        # In production, use a proper lookup table
        file_num = (hash(text) % 100) + 1
        
        print(f"🔊 {text}")
        self._send_command(0x03, file_num)  # Play file
        
        if DEBUG:
            print(f"[DFPLAYER] Playing file {file_num:04d}.mp3")
    
    def stop(self):
        """Stop playback."""
        self._send_command(0x16)


# =============================================================================
# Factory Function
# =============================================================================

def create_audio_player(backend: str = "print") -> AudioPlayer:
    """
    Create an audio player with the specified backend.
    
    Args:
        backend: "print" (debug), "dfplayer", or "i2s"
        
    Returns:
        AudioPlayer instance
    """
    if backend == "print":
        return AudioPlayer()
    
    elif backend == "dfplayer":
        from config import DFPLAYER_TX_PIN, DFPLAYER_RX_PIN
        return DFPlayerAudio(DFPLAYER_TX_PIN, DFPLAYER_RX_PIN)
    
    # Add more backends as needed (I2S, PWM, etc.)
    
    else:
        print(f"[AUDIO] Unknown backend '{backend}', falling back to print")
        return AudioPlayer()
