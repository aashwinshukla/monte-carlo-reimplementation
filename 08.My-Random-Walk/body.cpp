// Random Walk Simulator — terminal entry point
// Flow: pick particle count → pick step count → simulate → results → repeat or back

#include <iostream>
#include <limits>
#include <cmath>
using namespace std;

#include "validity-check.h"
#include "walk.h"

// ─────────────────────────────────────────────────────────────────────
// Prints the results of a simulation run
// ─────────────────────────────────────────────────────────────────────
void printResults(const SimulationStats &stats) {
    cout << "\n======= Results =======\n";
    cout << "Particles           : " << stats.numParticles  << "\n";
    cout << "Steps per particle  : " << stats.numSteps      << "\n";
    cout << "Expected distance   : " << stats.expectedDist  << "  (theory: sqrt(steps))\n";
    cout << "Avg final distance  : " << stats.avgFinalDist  << "\n";
    cout << "Avg max distance    : " << stats.avgMaxDist    << "\n";

    // If only one particle, show its individual result too
    if (stats.numParticles == 1) {
        cout << "\nParticle 1:\n";
        cout << "  Final distance : " << stats.walks[0].finalDist << "\n";
        cout << "  Max distance   : " << stats.walks[0].maxDist   << "\n";
    } else {
        cout << "\nPer-particle breakdown:\n";
        for (int i = 0; i < (int)stats.walks.size(); i++) {
            cout << "  Particle " << (i + 1)
                 << "  final: " << stats.walks[i].finalDist
                 << "  max: "   << stats.walks[i].maxDist  << "\n";
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// Asks for step count, runs simulation, offers repeat
// ─────────────────────────────────────────────────────────────────────
void runWithSteps(WalkSimulation &sim, int numParticles) {
    while (true) {
        cout << "\n--- Step Count ---\n";
        cout << "1. 100 steps\n";
        cout << "2. 1,000 steps\n";
        cout << "3. 10,000 steps\n";
        cout << "4. 100,000 steps\n";
        cout << "5. Custom\n";
        cout << "Choose: ";

        int input;
        cin >> input;
        int choice = InputValidator::checkSteps(input);
        if (choice == -1) continue;

        int steps = 0;
        if      (choice == 1) steps = 100;
        else if (choice == 2) steps = 1000;
        else if (choice == 3) steps = 10000;
        else if (choice == 4) steps = 100000;
        else if (choice == 5) {
            cout << "Enter number of steps: ";
            cin >> steps;
            if (cin.fail() || steps <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid step count.\n";
                continue;
            }
        }

        // Run and print
        SimulationStats stats = sim.run(numParticles, steps);
        printResults(stats);

        // Repeat or back
        while (true) {
            cout << "\n1. Run again\n2. Back\n";
            int r;
            cin >> r;
            int again = InputValidator::checkRunAgain(r);
            if (again == 1) {
                stats = sim.run(numParticles, steps);
                printResults(stats);
            } else if (again == 2) {
                return;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// Main — particle count menu
// ─────────────────────────────────────────────────────────────────────
int main() {
    WalkSimulation sim;

    while (true) {
        cout << "\n======= Random Walk Simulator =======\n";
        cout << "1. 1 Particle\n";
        cout << "2. 5 Particles\n";
        cout << "3. 10 Particles\n";
        cout << "4. 100 Particles\n";
        cout << "5. Custom\n";
        cout << "6. Exit\n";
        cout << "Choose: ";

        int input;
        cin >> input;
        int choice = InputValidator::checkMenu(input);
        if (choice == -1) continue;

        if (choice == 6) {
            cout << "Exiting.\n";
            break;
        }

        int particles = 0;
        if      (choice == 1) particles = 1;
        else if (choice == 2) particles = 5;
        else if (choice == 3) particles = 10;
        else if (choice == 4) particles = 100;
        else if (choice == 5) {
            cout << "Enter number of particles: ";
            cin >> particles;
            if (cin.fail() || particles <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid particle count.\n";
                continue;
            }
        }

        runWithSteps(sim, particles);
    }

    return 0;
}
