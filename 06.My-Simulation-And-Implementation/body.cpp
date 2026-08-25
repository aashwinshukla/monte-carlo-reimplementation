// Monte Carlo PI Estimator — terminal entry point
// Instantiates Simulator and InputValidator, drives the menu loop.

#include <iostream>
#include <limits>
using namespace std;

#include "validity-check.h"
#include "tests.h"

// Runs one simulation, then loops on the run-again prompt.
static void runWithRepeat(Simulator &sim, int trials) {
    RunResult r = sim.run(trials);
    cout << "\nTrials              : " << r.stats.trials
         << "\nPoints inside circle: " << r.stats.inCircle
         << "\nEstimated PI        : " << r.stats.estimatedPi
         << "\nActual PI           : 3.14159265358979"
         << "\nAbsolute Error      : " << r.stats.error
         << "\nTheoretical Bound   : " << r.stats.theoreticalBound
         << "\n";

    while (true) {
        cout << "\n1. Run again\n2. Back\n";
        int input;
        cin >> input;
        if (InputValidator::checkRunAgain(input) == 1)
            r = sim.run(trials);
        else if (InputValidator::checkRunAgain(input) == 2)
            break;
    }
}

int main() {
    Simulator sim;

    while (true) {
        cout << "\n======= Monte Carlo PI Estimator =======\n"
             << "1. Experiment Mode (all sizes)\n"
             << "2. 10 Trials\n"
             << "3. 100 Trials\n"
             << "4. 1,000 Trials\n"
             << "5. 10,000 Trials\n"
             << "6. 100,000 Trials\n"
             << "7. 1,000,000 Trials\n"
             << "8. Custom Trials\n"
             << "9. Exit\n"
             << "\nEnter choice: ";

        int input;
        cin >> input;
        int choice = InputValidator::checkMenu(input);
        if (choice == -1) continue;

        if (choice == 9) {
            cout << "Exiting.\n";
            break;

        } else if (choice == 1) {
            cout << "\n--- Experiment Mode ---\n";
            RunResult (*fns[])(Simulator&) = {
                [](Simulator &s){ return s.run10();   },
                [](Simulator &s){ return s.run100();  },
                [](Simulator &s){ return s.run1K();   },
                [](Simulator &s){ return s.run10K();  },
                [](Simulator &s){ return s.run100K(); },
                [](Simulator &s){ return s.run1M();   },
            };
            for (auto fn : fns) {
                RunResult r = fn(sim);
                cout << "Trials: " << r.stats.trials
                     << "  PI: "   << r.stats.estimatedPi
                     << "  Err: "  << r.stats.error << "\n";
            }

        } else if (choice == 8) {
            cout << "Enter number of trials: ";
            int custom;
            cin >> custom;
            if (cin.fail() || custom <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid trial count.\n";
                continue;
            }
            runWithRepeat(sim, custom);

        } else {
            static const int sizes[] = { 0, 0, 10, 100, 1000, 10000, 100000, 1000000 };
            runWithRepeat(sim, sizes[choice]);
        }
    }

    return 0;
}
