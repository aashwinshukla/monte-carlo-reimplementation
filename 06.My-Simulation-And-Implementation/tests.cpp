#include "tests.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────
// Simulator — implementation
// ─────────────────────────────────────────────────────────────────────

Simulator::Simulator()
    : m_gen(std::random_device{}())
    , m_dist(-1.0, 1.0)
{}

RunResult Simulator::run(int total_trials) {
    if (total_trials <= 0) total_trials = 1;

    int in_circle  = 0;
    int storeEvery = (total_trials > MAX_SCATTER_PTS)
                     ? total_trials / MAX_SCATTER_PTS
                     : 1;

    ScatterData scatter;
    scatter.inside.reserve(MAX_SCATTER_PTS);
    scatter.outside.reserve(MAX_SCATTER_PTS);

    for (int i = 0; i < total_trials; i++) {
        double x   = m_dist(m_gen);
        double y   = m_dist(m_gen);
        bool   hit = (x * x + y * y) <= 1.0;

        if (hit) in_circle++;

        if (i % storeEvery == 0) {
            if (hit) scatter.inside.push_back({x, y});
            else     scatter.outside.push_back({x, y});
        }
    }

    double estimated = (static_cast<double>(in_circle) / total_trials) * 4.0;

    SimResult stats;
    stats.trials           = total_trials;
    stats.inCircle         = in_circle;
    stats.estimatedPi      = estimated;
    stats.error            = std::abs(estimated - ACTUAL_PI);
    stats.theoreticalBound = 1.0 / std::sqrt(static_cast<double>(total_trials));

    return RunResult{ stats, scatter };
}

RunResult Simulator::run10()   { return run(10);      }
RunResult Simulator::run100()  { return run(100);     }
RunResult Simulator::run1K()   { return run(1000);    }
RunResult Simulator::run10K()  { return run(10000);   }
RunResult Simulator::run100K() { return run(100000);  }
RunResult Simulator::run1M()   { return run(1000000); }
