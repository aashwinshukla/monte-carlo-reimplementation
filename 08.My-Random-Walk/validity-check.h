#pragma once
#include <iostream>
#include <limits>
using namespace std;

// ─────────────────────────────────────────────────────────────────────
// InputValidator
// Static utility class — no state, just validation.
// Returns the valid value or -1 on bad input.
// ─────────────────────────────────────────────────────────────────────

class InputValidator {
public:
    // Main menu: particle count choice (1-6)
    static int checkMenu(int &input);

    // Steps menu: step count choice (1-5)
    static int checkSteps(int &input);

    // Run-again prompt (1 or 2)
    static int checkRunAgain(int &input);

private:
    static bool recoverCin(const char *msg);
};
