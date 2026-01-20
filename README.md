# 🕹️ The "Sweet Spot" Survival Arduino-Based Mini Game

A reflex-based Arduino mini-game built for the AUK Student Club Fair. Unlike standard distance sensors, this project gamifies control theory by challenging players to find and hold an invisible, moving target in mid-air.

## 🎮 Game Mechanics
The goal is simple: **Find the Green Zone and hold it.**
But there's a catch—the target moves, and you are running out of time.

1.  **The Invisible Target:** The system selects a random "Sweet Spot" between 10cm and 30cm.
2.  **The Search:**
    * 🔴 **Red LED:** You are lost (Death timer is ticking!).
    * 🟡 **Yellow LED:** Getting closer.
    * 🟢 **Green LED:** Target Locked!
3.  **The Drift (Difficulty Spike):** Once you find the sweet spot, you must hold your hand there for **5 seconds** to win. *However*, while you hold it, the target slowly "drifts" away from you, forcing you to adjust your hand in real-time.
4.  **Survival Mode:** If you stay out of the Sweet Spot for more than 5 seconds total, it's **GAME OVER**.

## 🛠️ Hardware List
* **Arduino Uno** (or compatible board)
* **HC-SR04** Ultrasonic Sensor
* **Piezo Buzzer** (Active or Passive)
* **LEDs:** 1x Green, 1x Yellow, 1x Red
* **Resistors:** 220Ω (x3 for LEDs)
* **Breadboard & Jumper Wires**

## 🔌 Circuit Diagram
![Circuit Diagram for the project](CircuitDiagram.png)

## ⚙️ The Code Logic
The project uses `millis()` timers to handle multitasking without blocking the processor.
* **Signal Processing:** Uses a weighted moving average filter (`0.7 * old + 0.3 * new`) to smooth out sensor jitter.
* **State Machine:** Dynamic switching between "Search," "Locked," "Victory," and "Death" states.
* **Audio:** Custom `tone()` frequencies simulate Mario-style startup, stress-building ticks, and victory melodies.

## 🚀 How to Install
1.  Clone this repository.
2.  Open `DistanceMiniGame.ino` in the Arduino IDE.
3.  Connect your Arduino Uno.
4.  Select your board and port, then click **Upload**.
5.  *Listen for the startup sound... and Good Luck!*
