#include "validity-check.h"

int check1(int &input1){
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number." << endl;
        return -1;
    }
    if (input1 >= 1 && input1 <= 9) return input1;
    cout << "Enter valid input! (1-9)" << endl;
    return -1;
}

int check2(int &input2){
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter 1 or 2." << endl;
        return -1;
    }
    if (input2 == 1) return 1;
    if (input2 == 2) return 2;
    cout << "Enter valid input! (1 or 2)" << endl;
    return -1;
}
