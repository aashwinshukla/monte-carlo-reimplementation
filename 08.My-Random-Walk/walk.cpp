#include "walk.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────
// Walker
// ─────────────────────────────────────────────────────────────────────

Walker::Walker()
    : m_x(0.0), m_y(0.0), m_maxDist(0.0), m_stepCount(0), m_storeEvery(1)
{
    // Start position is always the origin
    m_path.push_back({0.0, 0.0});
}

void Walker::step(std::mt19937 &gen) {
    // Pick one of 4 directions with equal probability
    std::uniform_int_distribution<int> dir(0, 3);
    int d = dir(gen);

    if      (d == 0) m_y += 1.0;   // North
    else if (d == 1) m_y -= 1.0;   // South
    else if (d == 2) m_x += 1.0;   // East
    else             m_x -= 1.0;   // West

    m_stepCount++;

    // Track max distance
    double dist = distanceFromOrigin();
    if (dist > m_maxDist) m_maxDist = dist;

    // Store path point at the chosen interval
    if (m_stepCount % m_storeEvery == 0)
        m_path.push_back({m_x, m_y});
}

double Walker::distanceFromOrigin() const {
    return std::sqrt(m_x * m_x + m_y * m_y);
}

void Walker::updatePath() {
    // Not used externally — storage rate is set before walking starts
}

// ─────────────────────────────────────────────────────────────────────
// WalkSimulation
// ─────────────────────────────────────────────────────────────────────

WalkSimulation::WalkSimulation()
    : m_gen(std::random_device{}())
{}

SimulationStats WalkSimulation::run(int numParticles, int numSteps) {
    SimulationStats stats;
    stats.numParticles = numParticles;
    stats.numSteps     = numSteps;
    stats.expectedDist = std::sqrt(static_cast<double>(numSteps));

    double totalFinal = 0.0;
    double totalMax   = 0.0;

    // Figure out how often to store path points so we don't exceed MAX_PATH_PTS
    int storeEvery = (numSteps > Walker::MAX_PATH_PTS)
                     ? numSteps / Walker::MAX_PATH_PTS
                     : 1;

    for (int p = 0; p < numParticles; p++) {
        Walker walker;
        walker.m_storeEvery = storeEvery;

        // Walk numSteps steps
        for (int s = 0; s < numSteps; s++) {
            walker.step(m_gen);
        }

        WalkResult result;
        result.steps     = numSteps;
        result.finalDist = walker.distanceFromOrigin();
        result.maxDist   = walker.maxDistance();
        result.path      = walker.path();

        totalFinal += result.finalDist;
        totalMax   += result.maxDist;

        stats.walks.push_back(result);
    }

    stats.avgFinalDist = totalFinal / numParticles;
    stats.avgMaxDist   = totalMax   / numParticles;

    return stats;
}
