#pragma once
#include <vector>
#include <utility>
#include <random>

// ─────────────────────────────────────────────────────────────────────
// WalkResult
// Plain data — one result per particle.
// ─────────────────────────────────────────────────────────────────────
struct WalkResult {
    int    steps        = 0;
    double finalDist    = 0.0;   // distance from origin at the end
    double maxDist      = 0.0;   // furthest point reached during walk

    // Path stored as (x, y) positions — capped for rendering
    std::vector<std::pair<double, double>> path;
};

// ─────────────────────────────────────────────────────────────────────
// SimulationStats
// Aggregate stats across all particles in one run.
// ─────────────────────────────────────────────────────────────────────
struct SimulationStats {
    int    numParticles  = 0;
    int    numSteps      = 0;
    double avgFinalDist  = 0.0;
    double avgMaxDist    = 0.0;
    double expectedDist  = 0.0;   // theoretical: sqrt(steps)

    // Individual particle results (one per particle)
    std::vector<WalkResult> walks;
};

// ─────────────────────────────────────────────────────────────────────
// Walker
// Represents one particle. Moves N/S/E/W one step at a time.
// Tracks its own position and path.
// ─────────────────────────────────────────────────────────────────────
class Walker {
public:
    Walker();

    // Take one random step — N, S, E, or W with equal probability.
    void step(std::mt19937 &gen);

    // Distance from origin at the current position.
    double distanceFromOrigin() const;

    // Current position.
    double x() const { return m_x; }
    double y() const { return m_y; }

    // Full path taken so far (sampled if steps > MAX_PATH_PTS).
    const std::vector<std::pair<double, double>> &path() const { return m_path; }

    // Furthest distance reached at any point during the walk.
    double maxDistance() const { return m_maxDist; }

private:
    double m_x       = 0.0;
    double m_y       = 0.0;
    double m_maxDist = 0.0;

    // Path — only every Nth step is stored to keep memory sane.
    std::vector<std::pair<double, double>> m_path;
    int m_stepCount  = 0;
    int m_storeEvery = 1;

    static const int MAX_PATH_PTS = 3000;

    void updatePath();

    // WalkSimulation needs to set the storage rate before walking
    friend class WalkSimulation;
};

// ─────────────────────────────────────────────────────────────────────
// WalkSimulation
// Owns the RNG. Runs numParticles walkers for numSteps each.
// ─────────────────────────────────────────────────────────────────────
class WalkSimulation {
public:
    WalkSimulation();

    // Run the simulation and return aggregate stats + all walks.
    SimulationStats run(int numParticles, int numSteps);

private:
    std::mt19937 m_gen;
};
