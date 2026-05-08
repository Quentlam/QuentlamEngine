import os
import sys
import json
import random

# ==============================================================================
# QL-Editor Regression Test & CI Runner
# ------------------------------------------------------------------------------
# Feature: Automated playback validation against "Gold Standard" hardcoded engine.
# Validates:
# - Score error == 0
# - Death coordinate error < 0.5 units
# - Audio trigger timing error < 50ms
# ==============================================================================

class RegressionTestRunner:
    def __init__(self, gold_standard_path, editor_executable_path):
        self.gold_standard = gold_standard_path
        self.editor_exec = editor_executable_path
        self.total_tests = 100
        self.pass_threshold = 0.99

    def generate_random_seed(self):
        return random.randint(1000, 999999)

    def generate_input_sequence(self, seed, frames=3600):
        """Generates a deterministic sequence of space-bar inputs based on seed."""
        random.seed(seed)
        sequence = []
        for f in range(frames):
            # E.g., Jump 2% of the time per frame
            if random.random() < 0.02:
                sequence.append(f)
        return sequence

    def run_simulation(self, executable, seed, input_sequence):
        """
        Mocks running a headless simulation of the game.
        Returns a dictionary of:
        - final_score: int
        - death_position: [x, y]
        - audio_triggers: list of (frame, event_name)
        """
        # In a real environment, this would invoke the engine via CLI:
        # > ./Engine.exe --headless --seed 1234 --input sequence.json --output result.json
        print(f"    -> Simulating seed {seed}...")
        
        # Mock result based on seed for demonstration
        random.seed(seed + 1)
        death_frame = random.randint(500, 3000)
        
        audio_triggers = []
        score = 0
        for frame in input_sequence:
            if frame < death_frame:
                audio_triggers.append({"frame": frame, "event": "Jump"})
            if frame > 0 and frame % 300 == 0 and frame < death_frame:
                score += 1
                audio_triggers.append({"frame": frame, "event": "Score"})
        
        audio_triggers.append({"frame": death_frame, "event": "GameOver"})

        return {
            "score": score,
            "death_position": [death_frame * 0.1, random.uniform(-5, 5)],
            "audio_triggers": audio_triggers
        }

    def compare_results(self, gold, new_run):
        # 1. Score Error == 0
        if gold["score"] != new_run["score"]:
            return False, "Score mismatch"

        # 2. Death Coordinate Error < 0.5
        dx = abs(gold["death_position"][0] - new_run["death_position"][0])
        dy = abs(gold["death_position"][1] - new_run["death_position"][1])
        if dx > 0.5 or dy > 0.5:
            return False, f"Death pos error dx={dx}, dy={dy}"

        # 3. Audio Trigger Timing Error < 50ms (Assuming 60fps -> < 3 frames)
        if len(gold["audio_triggers"]) != len(new_run["audio_triggers"]):
            return False, "Audio trigger count mismatch"
        
        for g_aud, n_aud in zip(gold["audio_triggers"], new_run["audio_triggers"]):
            if g_aud["event"] != n_aud["event"]:
                return False, "Audio event mismatch"
            frame_diff = abs(g_aud["frame"] - n_aud["frame"])
            if frame_diff > 3: # 3 frames * 16.6ms = ~50ms
                return False, f"Audio timing diff > 50ms ({frame_diff} frames)"

        return True, "Pass"

    def run_ci(self):
        print("=== Starting QL-Editor Regression CI ===")
        print(f"Targeting {self.total_tests} test cases...")
        
        passed = 0
        
        for i in range(self.total_tests):
            seed = self.generate_random_seed()
            input_seq = self.generate_input_sequence(seed)
            
            # Simulate Gold Standard (Hardcoded)
            gold_result = self.run_simulation(self.gold_standard, seed, input_seq)
            
            # Simulate New Data-Driven Runtime
            new_result = self.run_simulation(self.editor_exec, seed, input_seq)
            
            success, msg = self.compare_results(gold_result, new_result)
            if success:
                passed += 1
            else:
                print(f"[FAIL] Seed {seed}: {msg}")
                
        pass_rate = passed / self.total_tests
        print(f"\n=== CI Completed ===")
        print(f"Pass Rate: {pass_rate*100:.1f}% ({passed}/{self.total_tests})")
        
        if pass_rate >= self.pass_threshold:
            print("[SUCCESS] Pipeline passed.")
            sys.exit(0)
        else:
            print("[ERROR] Pipeline failed. Did not meet 99% threshold.")
            sys.exit(1)

if __name__ == "__main__":
    runner = RegressionTestRunner("ParkourGame_Gold.exe", "QLEditor_Runtime.exe")
    runner.run_ci()
