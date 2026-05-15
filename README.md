<!-- HEADER -->
<div align="center">

# Robotic Arm Link Design & Motor-Gearbox Selection System

**C++ engineering toolkit for robotic joint structural analysis and optimal actuator selection**

[![Language](https://img.shields.io/badge/Language-C%2B%2B11-blue?style=flat-square&logo=cplusplus)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Scraper-Python%203-yellow?style=flat-square&logo=python)](https://www.python.org/)
[![Data Source](https://img.shields.io/badge/Data-Maxon%20Group-red?style=flat-square)](https://www.maxongroup.com/)
[![Motors](https://img.shields.io/badge/Motors-501-green?style=flat-square)]()
[![Gearboxes](https://img.shields.io/badge/Gearboxes-501-green?style=flat-square)]()
[![Pairs](https://img.shields.io/badge/Combinations-251%2C001-orange?style=flat-square)]()
[![License](https://img.shields.io/badge/License-Academic-lightgrey?style=flat-square)]()

</div>

---

## Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [System Architecture](#-system-architecture)
- [Engineering Logic](#-engineering-logic)
- [Data Pipeline](#-data-pipeline)
- [Project Structure](#-project-structure)
- [Installation & Build](#-installation--build)
- [Usage](#-usage)
- [Code Reference](#-code-component-reference)
- [Algorithm Details](#-algorithm-details)
- [Limitations](#-limitations)

---

## Overview

This project solves two interconnected problems in robotic arm design:

1. **Structural sizing**: Given a link's material, geometry, payload, and motion profile, compute the bending stress and iteratively optimize the cross-section dimensions until stress exactly meets (but never exceeds) the material's yield strength.

2. **Actuator selection**: Using a pre-built database of 501 real Maxon motors and 501 real Maxon gearboxes, enumerate all ~251,000 motor-gearbox combinations, compute each pair's output torque and speed, and surface candidates that satisfy the joint's required torque.

The component databases are populated automatically by a Python scraping pipeline that crawls Maxon Group's online catalog with Selenium and BeautifulSoup.

---

## Features

- **Material database** with interactive CLI: select from existing entries or add custom materials (name, yield strength, density)
- **Bending stress engine** for both circular and rectangular beam cross-sections
- **Iterative cross-section optimizer**: scales dimensions ±1% per iteration until stress converges within 1% of yield strength
- **Full combinatorial pairing**: O(N×M) enumeration of all motor-gearbox combinations, persisted to CSV
- **Required torque calculator** combining gravitational and inertial loading
- **Smart unit formatting** (`dimPrecision`) Displays output values in scientific notation
- **Python scraper** with retry logic, random delay jitter, and fault-tolerant row parsing
- **Extensible CSV architecture**: all databases can be extended at runtime without recompilation

---

## Engineering Logic

<details>
<summary><b>Bending Moment Model</b></summary>

For a cantilever robotic link of length L rotating about its root joint:

```
M = m_L · g · (L/2)     ← gravity at link CoM
  + m_p · g · L          ← gravity at payload tip
  + m_L · (L/2)² · α    ← inertial load at link CoM
  + m_p · L²  · α       ← inertial load at payload
```

| Symbol | Meaning | Unit |
|--------|---------|------|
| m_L | Link mass (derived from geometry + density) | kg |
| m_p | Payload mass (user input) | kg |
| g | Gravitational acceleration (9.81) | m/s² |
| L | Link length | m |
| α | Angular acceleration | rad/s² |

</details>

<details>
<summary><b>Bending Stress Equations</b></summary>

**Circular cross-section (radius r)**

```
I_circ   = π · r⁴ / 4
σ_circ   = M · r / I_circ
m_L      = ρ · π · r² · L
```

**Rectangular cross-section (width b × height h)**

```
I_rect   = b · h³ / 12
σ_rect   = M · (h/2) / I_rect
m_L      = ρ · b · h · L
```

</details>

<details>
<summary><b>Cross-Section Optimizer</b></summary>

The optimizer scales the cross-section iteratively until the computed stress lies in the convergence band `[0.99·σ_y, σ_y]`:

```
while not converged:
    σ = compute_stress()
    if σ > σ_y:
        dim(s) *= 1.01    # grow section
    elif σ < σ_y:
        dim(s) *= 0.99    # shrink section
    if 0.99·σ_y ≤ σ ≤ σ_y:
        converged = True
```

This guarantees full material utilization — the section carries the maximum allowable load without yielding.

</details>

<details>
<summary><b>Motor-Gearbox Output Equations</b></summary>

```
T_output = T_motor × reduction_ratio × (η / 100)
ω_output = ω_motor / reduction_ratio
```

| Symbol | Meaning | Unit |
|--------|---------|------|
| T_motor | Motor nominal torque | mNm |
| η | Gearbox max efficiency | % |
| reduction_ratio | Absolute gear reduction | — |
| T_output | Joint output torque | mNm |
| ω_output | Joint output speed | rpm |

</details>

---

## Data Pipeline

```
GetProductsUrls()        — paginate catalog, collect URLs + diameter + width
        ↓
GetProductsDetails()     — visit each URL, scrape specification tables
        ↓
motorsDetails.json       — raw structured data
        ↓
convert_json_to_csv()    — flatten JSON to DataFrame
        ↓
CleanData()              — strip units from values, rename columns with units
        ↓
AddIndexColumn()         — prepend integer index
        ↓
motorsDetailsCleanedIndexed.csv  →  motors.csv / gears.csv
```

**Robustness features:**
- Up to 10 retries per product page
- Random sleep jitter (1–3 s) between requests to avoid rate-limiting
- Per-row exception handling; malformed rows are skipped, not fatal

---

## Project Structure

```
project/
 ├── RoboticArmOptimization.cpp          # C++ core — all classes and main()
 ├── GearBoxes_Scraper.py                # Python scraping + data cleaning for gearboxes
 |── Motors_Scraper.py                   # Python scraping + data cleaning for motors
 └── Data/
     └── Processed/
         ├── motors.csv                  # 501 Maxon motors
         ├── gears.csv                   # 501 Maxon gearboxes
         ├── materials.csv               # 9 engineering materials
         └── motor_gearbox_pairs.csv     # ~251,000 pre-computed pairs
```

---

## Installation & Build

### C++ Engine

**Requirements:** C++11 compiler (g++ ≥ 4.8, MSVC 2015+, or Clang ≥ 3.3)

```bash
# Clone / download project, then:
g++ -std=c++11 RoboticArmOptimization.cpp -o arm_design

# Windows (MSVC)
cl /EHsc /std:c++11 RoboticArmOptimization.cpp /Fe:arm_design.exe
```

```bash
# Run
./arm_design          # Linux / macOS
arm_design.exe        # Windows
```

> The executable must be run from the project root so that relative paths like `./Data/Processed/motors.csv` resolve correctly.

### Python Scraper

> **The data files are already provided. Only run the scraper if you need to refresh the database.**

```bash
pip install selenium beautifulsoup4 lxml pandas
```

Update `driver_path` in both `GearBoxes_Scraper.py` and `Motors_Scraper.py` to your ChromeDriver location, then:

```python
# Run in sequence inside Scraper.py
data = GetProductsUrls()
GetProductsDetails(data)
convert_json_to_csv()
CleanData()
AddIndexColumn()
```

---

## Code Component Reference

| Component | File | Responsibility |
|-----------|------|---------------|
| `csvHandler` | RoboticArmOptimization.cpp | Low-level CSV read (fstream) and append operations |
| `Material` | RoboticArmOptimization.cpp | Material DB: parse CSV, interactive add/select, input validation |
| `StressAnalysis` | RoboticArmOptimization.cpp | Bending stress for circular + rectangular; iterative optimizer; inherits Material |
| `Motor` | RoboticArmOptimization.cpp | Motor DB: parse CSV, interactive add |
| `Gearbox` | RoboticArmOptimization.cpp | Gearbox DB: parse CSV with fractional ratio parsing (e.g. `57/13`) |
| `MotorGearboxPair` | RoboticArmOptimization.cpp | Enumerate N×M combinations; read/write pairs CSV |
| `MotorGearboxOptimization` | RoboticArmOptimization.cpp | Required torque calculation; inherits StressAnalysis |
| `dimPrecision()` | RoboticArmOptimization.cpp | Auto-scaling unit formatter for length, pressure, mass |

---

## Limitations

- Stress model is a cantilever beam approximation; torsion and axial loads are not considered.
- Gearbox efficiency is taken as constant (catalog peak value); true efficiency varies with speed and load.
- Optimizer step size is fixed at 1%; very tight convergence bands may oscillate.
- The scraper targets Maxon's current HTML layout; site redesigns require selector updates.
- `motor_gearbox_pairs.csv` must be regenerated manually if base databases change.
- `main()` currently lists existing pairs only; the full optimization workflow requires calling `MotorGearboxOptimization` (partially implemented).
