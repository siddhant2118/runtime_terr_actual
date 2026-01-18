## Bug Report: Audio Looping / Resetting
**Issue:** The ESP32 receives the `/detect?type=saw_human` command continuously (every frame) while a human is visible. This causes the audio track on the robot to restart repeatedly (stuttering/looping) instead of playing once completely.

**Request for App Developer:**
Please implement a **Cooldown** or **Blocker** logic:
1.  When a Human is detected -> Send `/detect?type=saw_human` **ONCE**.
2.  **Start a Timer/Flag:** Do NOT send this specific command again for **5 Seconds** (or until the audio file duration typically finishes).
3.  **Allow other commands:** (e.g., specific voice commands) can still pass through, but the visual detection event needs to be rate-limited.

**Example Logic:**
```python
if human_detected and (current_time - last_sent_time > 5000):
    send_request("/detect?type=saw_human")
    last_sent_time = current_time
```
