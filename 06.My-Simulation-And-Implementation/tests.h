#pragma once
#include <random>
#include <vector>
#include <utility>

// ── Data structures ──────────────────────────────────────────────────

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
    SimResult  stats;
    ScatterData scatter;
};

// ── API ──────────────────────────────────────────────────────────────

RunResult runSimulation(std::mt19937 &gen, int total_trials);

RunResult test1(std::mt19937 &gen);   //        10
RunResult test2(std::mt19937 &gen);   //       100
RunResult test3(std::mt19937 &gen);   //     1,000
RunResult test4(std::mt19937 &gen);   //    10,000
RunResult test5(std::mt19937 &gen);   //   100,000
RunResult test6(std::mt19937 &gen);   // 1,000,000

RunResult testCustom(std::mt19937 &gen, int trials);
