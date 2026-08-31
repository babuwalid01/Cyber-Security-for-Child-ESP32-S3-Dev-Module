# 🛡️✨ SafeKids & Privacy-First Hardware DNS Shield

> 🎯 **Our Ultimate Mission:**
> *"At the end of the day, the future of tomorrow's children is safe in our hands. We will ensure their secure and clean internet access!"*

---

## 📌 Project Overview
* **Developer:** Walid Ul Islam (Self-Educated Independent Researcher in Embedded Systems & Cyber Security)
* **Target Hardware:** ESP32-S3 (Dual-Core, PSRAM Supported)
* **Real-Time Clock:** DS1302 Hardware RTC Module
* **Firmware Version:** `esp32S3-11.2.8.2-NET-ULTRA RTC Module`
* **Release Date:** August 31, 2026

---

## 🛠️ Key Hardware Features & Isolation Design
* **Optocoupler Night-Mode Isolation (`NIGHT_PIN` / GPIO 18):** 
  * Controls the ground lines of all status LEDs through an optocoupler.
  * Ensures zero voltage fluctuations and isolates common ground automatically during scheduled hours or idle periods.
* **Transistor-Driven Thermal Fan Control (`FAN_PIN` / GPIO 13):**
  * Monitors internal CPU core temperature via `temperatureRead()`.
  * Triggers an isolated 12V cooling fan driver when CPU temperature exceeds **49.5°C** and runs until cooled below **45.0°C**.
* **PSRAM Memory Allocation:**
  * Network packet buffers are allocated directly to PSRAM (`MALLOC_CAP_SPIRAM`), keeping internal SRAM free for real-time web dashboard handling.
* **Instant Blocking & Filtering:**
  * Intercepts DNS queries on UDP Port 53.
  * Instant-blocks adult/pornographic domains, harmful online gambling, and trackers while routing clean traffic via malware-protected upstream DNS (`1.1.1.3`).

---

## ⚖️ Disclaimer & Terms of Use
1. **Non-Commercial & Open Initiative:** Designed strictly for non-profit child safety and educational research.
2. **Provided "AS IS":** The code and hardware architecture are provided without implied warranties.
3. **Independent Operation:** Operates independently without affiliation to any commercial entity or government telecom authority.
