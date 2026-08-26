# Monte Carlo Simulation — C++

A from-scratch Monte Carlo simulator built in C++, covering probability theory, statistical analysis, and graphical visualisation. Two simulations implemented so far: PI estimation and Random Walk.

---

## What is Monte Carlo Simulation?

Monte Carlo simulation estimates unknown values by running large numbers of random experiments and observing what the results converge to. The core guarantee is the **Law of Large Numbers** — the more trials you run, the closer your observed frequency gets to the true probability.

The name comes from the Monte Carlo casino in Monaco. The method was developed in the 1940s by Stanislaw Ulam and John von Neumann at Los Alamos while working on neutron diffusion problems too complex to solve analytically but tractable through random sampling.

---

## Simulation 1 — PI Estimation

A unit square contains a quarter-circle of radius 1. Points thrown uniformly at random inside the square land inside the quarter-circle with probability π/4.

```
π ≈ 4 × (points inside circle / total points)
```

Condition for a point (x, y) to be inside:
```
x² + y² ≤ 1
```

Error in the estimate:
```
Error ≈ 1 / √N
```

**Features:**
- 6 pre-defined sizes — 10 to 1,000,000 trials
- Custom trial count
- Experiment mode — all sizes back to back with comparison table
- Scatter plot — green inside, red outside, cyan arc overlay

---

## Simulation 2 — Random Walk

A particle starts at the origin (0, 0). At each step it moves North, South, East, or West with equal probability (1/4 each). The key question: how far from the origin does it end up after N steps?

Intuition says the steps should cancel out and the particle should stay near the origin. The actual result is more interesting — the expected distance grows with the square root of steps:

```
Expected distance ≈ √N
```

After 100 steps   → expected distance ≈ 10  
After 10,000 steps → expected distance ≈ 100

This is Monte Carlo applied to geometry: instead of computing the expected distance analytically, simulate thousands of walks and let the Law of Large Numbers produce the answer.

**Features:**
- Choose particle count: 1, 5, 10, 100, or custom
- Choose step count: 100, 1K, 10K, 100K, or custom
- Per-particle path visualisation — colour-fading line from start (green) to end (red)
- Aggregate stats: avg final distance, avg max distance, theoretical √N
- Arrow through each particle's path individually in the UI

---

## Graphical UI

Built with **Dear ImGui + DirectX 9 + Win32** — opens in its own window, no console behind it.

Two tabs in a single window:

```
┌─────────────────────────────────────────────────────────┐
│  Monte Carlo Simulator                                  │
├──────────────────────────────────────────────────────── │
│  [ PI Estimator ]  [ Random Walk ]                      │
├──────────────┬───────────────────────┬──────────────────┤
│  MENU        │  CANVAS               │  RESULTS         │
│              │  (ImDrawList)         │                  │
└──────────────┴───────────────────────┴──────────────────┘
```

Controls: click buttons or use arrow keys + Enter to navigate, R to repeat last run, Esc to go back.

---

## Project Structure

```
01.Introduction/                    — What Monte Carlo is and why it works
02.Monte-Carlo-Roulette/            — Roulette sim, Law of Large Numbers, Gambler's Fallacy
03.Regression-To-The-Mean/          — Galton's observation and statistical regression
04.Casinos-Are-Unfair/              — How the house edge works mathematically
05.Variance-And-Standard-Deviation/ — Variance, SD, empirical rule, confidence intervals
06.My-Simulation-And-Implementation/— PI estimator C++ + ImGui UI (main window)
07.Random-Walk-Understanding/       — Theory notes on random walks and Brownian motion
08.My-Random-Walk/                  — Random walk C++ implementation
```

---

## Build — UI (folder 06)

Requires: `g++`, `CMake`, `mingw32-make`, DirectX 9 (ships with Windows)

```bash
cd 06.My-Simulation-And-Implementation
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/mingw32-make.exe"
"C:/MinGW/bin/mingw32-make.exe" -j4
.\monte-carlo-pi.exe
```

## Build — Terminal only (folder 08)

```bash
cd 08.My-Random-Walk
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/mingw32-make.exe"
"C:/MinGW/bin/mingw32-make.exe" -j4
.\random-walk.exe
```

---

## Sample Output — PI

| Trials    | Estimated π   | Error      |
|-----------|---------------|------------|
| 10        | 2.800000000   | 0.341593   |
| 100       | 3.120000000   | 0.021593   |
| 1,000     | 3.156000000   | 0.014407   |
| 10,000    | 3.139200000   | 0.002393   |
| 100,000   | 3.142680000   | 0.001087   |
| 1,000,000 | 3.141596000   | 0.000004   |

## Sample Output — Random Walk (10 particles, 1000 steps)

| Particle | Final Dist | Max Dist |
|----------|------------|----------|
| 1        | 18.4       | 31.2     |
| 2        | 12.7       | 28.6     |
| 3        | 34.1       | 41.8     |
| ...      | ...        | ...      |
| Avg      | ~22.4      | ~33.1    |
| √1000    | 31.6       | —        |

---

## Architecture

- `Simulator` class — owns RNG, runs PI simulations, returns `RunResult`
- `Walker` class — one particle, tracks position and path
- `WalkSimulation` class — owns RNG, runs N walkers for S steps, returns `SimulationStats`
- `InputValidator` class — static validation methods for all menus

---

## What's Next

- **Dice probability simulator** — experimental vs theoretical probability comparison (sum of two dice across millions of rolls)

---

## References

- Guttag, J. — *Introduction to Computation and Programming Using Python* (MIT OCW)
- Huff, D. & Geis, I. — *How to Take a Chance*
- Ulam, S. — *Adventures of a Mathematician*
