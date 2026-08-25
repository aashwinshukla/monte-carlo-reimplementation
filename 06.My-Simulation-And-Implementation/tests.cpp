#include "tests.h"
#include <iostream>
#include <cmath>

// Helper — all tests share the same logic, only trial count differs
static void runTest(std::mt19937 &gen, int total_trials) {
    int in_circle = 0;
    std::uniform_real_distribution<double> realDist(-1.0, 1.0);

    for (int i = 0; i < total_trials; i++) {
        double x = realDist(gen);
        double y = realDist(gen);
        if ((x * x) + (y * y) <= 1.0) {
            in_circle++;
        }
    }

    double estimate = (static_cast<double>(in_circle) / total_trials) * 4.0;
    double error    = std::abs(estimate - 3.14159265358979323846);

    std::cout << "\n";
    std::cout << "Trials              : " << total_trials  << "\n";
    std::cout << "Points inside circle: " << in_circle     << "\n";
    std::cout << "Estimated PI        : " << estimate      << "\n";
    std::cout << "Actual PI           : " << 3.14159265358979323846 << "\n";
    std::cout << "Absolute Error      : " << error         << "\n";
    std::cout << "Theoretical Bound   : " << (1.0 / std::sqrt(total_trials)) << "\n";
}

void test1(std::mt19937 &gen) { runTest(gen, 10);      }
void test2(std::mt19937 &gen) { runTest(gen, 100);     }
void test3(std::mt19937 &gen) { runTest(gen, 1000);    }
void test4(std::mt19937 &gen) { runTest(gen, 10000);   }
void test5(std::mt19937 &gen) { runTest(gen, 100000);  }
void test6(std::mt19937 &gen) { runTest(gen, 1000000); }

void testCustom(std::mt19937 &gen, int trials) {
    if (trials <= 0) {
        std::cout << "Trial count must be greater than 0.\n";
        return;
    }
    runTest(gen, trials);
}
