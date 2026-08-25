// Monte Carlo PI Estimator
// Starting with the pi estimate as discussed in Chapter 1.

#include <iostream>
#include <random>
#include <limits>
using namespace std;

#include "validity-check.h"
#include "tests.h"

inline constexpr double PI = 3.14159265358979323846;

// Runs a test function then offers "Run again / Back" loop
static void runWithRepeat(void (*testFn)(mt19937&), mt19937 &gen) {
    testFn(gen);
    while (true) {
        cout << "\n1. Run again\n2. Back\n";
        int input2;
        cin >> input2;
        int choice2 = check2(input2);
        if (choice2 == 1) {
            testFn(gen);
        } else if (choice2 == 2) {
            break;
        }
        // if -1 (invalid), loop re-prompts naturally
    }
}

int main() {
    random_device rd;
    mt19937 gen(rd());

    while (true) {
        cout << "\n";
        cout << "======= Monte Carlo PI Estimator =======\n";
        cout << "1. Existing Tests (pre-defined run of all sizes)\n";
        cout << "2. 10 Trials\n";
        cout << "3. 100 Trials\n";
        cout << "4. 1,000 Trials\n";
        cout << "5. 10,000 Trials\n";
        cout << "6. 100,000 Trials\n";
        cout << "7. 1,000,000 Trials\n";
        cout << "8. Custom Trials\n";
        cout << "9. Exit\n";
        cout << "\nEnter choice: ";

        int input1;
        cin >> input1;
        int choice1 = check1(input1);

        if (choice1 == -1) continue;

        if (choice1 == 9) {
            cout << "Exiting.\n";
            break;

        } else if (choice1 == 1) {
            // Run all sizes back to back (Experiment Mode from Phase 5)
            cout << "\n--- Running all test sizes ---\n";
            test1(gen);
            test2(gen);
            test3(gen);
            test4(gen);
            test5(gen);
            test6(gen);

        } else if (choice1 == 2) {
            runWithRepeat(test1, gen);

        } else if (choice1 == 3) {
            runWithRepeat(test2, gen);

        } else if (choice1 == 4) {
            runWithRepeat(test3, gen);

        } else if (choice1 == 5) {
            runWithRepeat(test4, gen);

        } else if (choice1 == 6) {
            runWithRepeat(test5, gen);

        } else if (choice1 == 7) {
            runWithRepeat(test6, gen);

        } else if (choice1 == 8) {
            cout << "Enter number of trials: ";
            int customTrials;
            cin >> customTrials;
            if (cin.fail() || customTrials <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid trial count.\n";
                continue;
            }
            testCustom(gen, customTrials);
            while (true) {
                cout << "\n1. Run again\n2. Back\n";
                int input2;
                cin >> input2;
                int choice2 = check2(input2);
                if (choice2 == 1) {
                    testCustom(gen, customTrials);
                } else if (choice2 == 2) {
                    break;
                }
            }
        }
    }

    return 0;
}
