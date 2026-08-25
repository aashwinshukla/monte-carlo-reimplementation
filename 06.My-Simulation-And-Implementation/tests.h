#pragma once
#include <random>
#include <vector>
#include <utility>

// ─────────────────────────────────────────────────────────────────────
// DATA STRUCTURES
// Plain structs — just data, no behaviour.
// ─────────────────────────────────────────────────────────────────────

struct SimResult {
    int    trials           = 0;
    int    inCircle         = 0;
    double estimatedPi      = 0.0;
    double error            = 0.0;
    double theoreticalBound = 0.0;
};

struct ScatterData {
    std::vector<std::pair<double,double>> inside;
    std::vector<std::pair<double,double>> outside;
};

struct RunResult {
    SimResult   stats;
    ScatterData scatter;
};

// ─────────────────────────────────────────────────────────────────────
// Simulator
//
// Owns the RNG and knows how to run a Monte Carlo PI simulation.
// Constructed once, reused across multiple calls — the RNG state
// is preserved between runs so each run is truly independent.
// ─────────────────────────────────────────────────────────────────────

class Simulator {
public:
    // Seed from hardware entropy by default.
    Simulator();

    // Run a simulation with the given number of trials.
    // Returns the result + scatter points for visualisation.
    RunResult run(int total_trials);

    // Convenience wrappers for pre-defined sizes.
    RunResult run10();
    RunResult run100();
    RunResult run1K();
    RunResult run10K();
    RunResult run100K();
    RunResult run1M();

private:
    std::mt19937                            m_gen;
    std::uniform_real_distribution<double>  m_dist;

    static constexpr int    MAX_SCATTER_PTS = 2000;
    static constexpr double ACTUAL_PI       = 3.14159265358979323846;
};
