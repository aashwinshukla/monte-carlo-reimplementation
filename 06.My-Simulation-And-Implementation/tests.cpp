#include "tests.h"
#include <cmath>

// Maximum scatter points stored regardless of trial count.
// Keeps memory and render time sane for 1,000,000-trial runs.
static const int MAX_SCATTER_PTS = 2000;

RunResult runSimulation(std::mt19937 &gen, int total_trials) {
    int in_circle = 0;
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    // Decide how often to record a point for the scatter plot.
    // e.g. 1,000,000 trials → store every 500th point → 2000 points max.
    int storeEvery = (total_trials > MAX_SCATTER_PTS)
                     ? total_trials / MAX_SCATTER_PTS
                     : 1;

    ScatterData scatter;
    scatter.inside.reserve(MAX_SCATTER_PTS);
    scatter.outside.reserve(MAX_SCATTER_PTS);

    for (int i = 0; i < total_trials; i++) {
        double x = dist(gen);
        double y = dist(gen);
        bool hit = (x * x + y * y) <= 1.0;

        if (hit) in_circle++;

        // Record for scatter only at the chosen interval
        if (i % storeEvery == 0) {
            if (hit)
                scatter.inside.push_back({x, y});
            else
                scatter.outside.push_back({x, y});
        }
    }

    constexpr double ACTUAL_PI = 3.14159265358979323846;
    double estimated  = (static_cast<double>(in_circle) / total_trials) * 4.0;
    double error      = std::abs(estimated - ACTUAL_PI);
    double theoBound  = 1.0 / std::sqrt(static_cast<double>(total_trials));

    SimResult stats;
    stats.trials            = total_trials;
    stats.inCircle          = in_circle;
    stats.estimatedPi       = estimated;
    stats.error             = error;
    stats.theoreticalBound  = theoBound;

    return RunResult{ stats, scatter };
}

RunResult test1(std::mt19937 &gen) { return runSimulation(gen, 10);      }
RunResult test2(std::mt19937 &gen) { return runSimulation(gen, 100);     }
RunResult test3(std::mt19937 &gen) { return runSimulation(gen, 1000);    }
RunResult test4(std::mt19937 &gen) { return runSimulation(gen, 10000);   }
RunResult test5(std::mt19937 &gen) { return runSimulation(gen, 100000);  }
RunResult test6(std::mt19937 &gen) { return runSimulation(gen, 1000000); }

RunResult testCustom(std::mt19937 &gen, int trials) {
    if (trials <= 0) trials = 1;
    return runSimulation(gen, trials);
}
