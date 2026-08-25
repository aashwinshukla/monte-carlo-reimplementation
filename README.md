# Monte Carlo Simulation — C++

A from-scratch Monte Carlo simulator built in C++, starting with the classic π estimation problem and expanding into probability theory, statistical analysis, and graphical visualisation.

---

## What is Monte Carlo Simulation?

Monte Carlo simulation is a method of estimating unknown values by running large numbers of random experiments and observing what the results converge to. The core guarantee is the **Law of Large Numbers** — the more trials you run, the closer your observed frequency gets to the true probability.

The name comes from the Monte Carlo casino in Monaco. The method was developed in the 1940s by Stanislaw Ulam and John von Neumann at Los Alamos while working on neutron diffusion problems — problems too complex to solve analytically but tractable through random sampling.

---

## PI Estimation

The first simulation estimates π using random geometry.

A unit square contains a quarter-circle of radius 1. If points are thrown uniformly at random inside the square, the fraction that land inside the quarter-circle converges to π/4.

```
π ≈ 4 × (points inside circle / total points)
```

The condition for a point (x, y) to be inside the circle:

```
x² + y² ≤ 1
```

The error in the estimate follows:

```
Error ≈ 1 / √N
```

So accuracy improves with more trials — slowly, but guaranteed.

---

## Features

- **6 pre-defined simulation sizes** — 10 to 1,000,000 trials
- **Custom trial count** — run any number of trials
- **Experiment mode** — runs all sizes back to back and compares results in a table
- **Graphical UI** — built with Dear ImGui + DirectX 9, opens in its own window
- **Scatter plot** — visualises every sampled point, colour-coded inside (green) vs outside (red) with the quarter-circle arc overlaid in cyan
- **Results panel** — shows estimated π, actual π, absolute error, theoretical error bound, and an error progress bar
- **OOP architecture** — `Simulator` class owns the RNG and simulation logic, `InputValidator` handles input, clean separation of concerns

---

## Project Structure

```
01.Introduction/                  — What Monte Carlo is and why it works
02.Monte-Carlo-Roulette/          — Roulette simulation, Law of Large Numbers, Gambler's Fallacy
03.Regression-To-The-Mean/        — Galton's observation and statistical regression
04.Casinos-Are-Unfair/            — How the house edge works mathematically
05.Variance-And-Standard-Deviation/ — Quantifying uncertainty, empirical rule, confidence intervals
06.My-Simulation-And-Implementation/ — C++ implementation with ImGui UI
```

---

## Build

Requires: `g++`, `CMake`, `mingw32-make`, DirectX 9 (ships with Windows)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
.\monte-carlo-pi.exe
```

---

## Sample Output

| Trials    | Estimated π   | Error      |
|-----------|---------------|------------|
| 10        | 2.800000000   | 0.341593   |
| 100       | 3.120000000   | 0.021593   |
| 1,000     | 3.156000000   | 0.014407   |
| 10,000    | 3.139200000   | 0.002393   |
| 100,000   | 3.142680000   | 0.001087   |
| 1,000,000 | 3.141596000   | 0.000004   |

As trials increase, the estimate converges to the true value of π = 3.14159265...

---

## What's Next

- **Random Walk simulation** — a particle moving randomly in 2D, tracking path, distance statistics, and visualising the walk as a drawn path in the UI
- **Dice probability simulator** — experimental vs theoretical probability comparison

---

## References

- Guttag, J. — *Introduction to Computation and Programming Using Python* (MIT OCW lecture series)
- Huff, D. & Geis, I. — *How to Take a Chance*
- Ulam, S. — *Adventures of a Mathematician*
