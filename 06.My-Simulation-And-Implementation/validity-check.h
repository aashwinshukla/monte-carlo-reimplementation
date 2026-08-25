#pragma once
#include <iostream>
#include <limits>
using namespace std;

// ─────────────────────────────────────────────────────────────────────
// InputValidator
//
// Static utility class — no state, just validation logic.
// All methods return the validated value or -1 on bad input.
// ─────────────────────────────────────────────────────────────────────

class InputValidator {
public:
    // Validates a main menu choice (1-9).
    static int checkMenu(int &input);

    // Validates a run-again prompt (1 or 2).
    static int checkRunAgain(int &input);

private:
    // Checks if cin is in a failed state and recovers it.
    // Returns true if cin was bad (caller should return -1).
    static bool recoverCin(const char *msg);
};
