#include "validity-check.h"

bool InputValidator::recoverCin(const char *msg) {
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << msg << endl;
        return true;
    }
    return false;
}

int InputValidator::checkMenu(int &input) {
    if (recoverCin("Invalid input! Please enter a number."))
        return -1;
    if (input >= 1 && input <= 9)
        return input;
    cout << "Enter valid input! (1-9)" << endl;
    return -1;
}

int InputValidator::checkRunAgain(int &input) {
    if (recoverCin("Invalid input! Please enter 1 or 2."))
        return -1;
    if (input == 1 || input == 2)
        return input;
    cout << "Enter valid input! (1 or 2)" << endl;
    return -1;
}
