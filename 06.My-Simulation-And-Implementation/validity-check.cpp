int check1(int &input1){
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid Input! Please enter a number." << endl;
        return 0;
    }
    if      (input1 == 1) return 1;
    else if (input1 == 2) return 2;
    else if (input1 == 3) return 3;
    else if (input1 == 4) return 4;
    else if (input1 == 5) return 5;
    else if (input1 == 6) return 6;
    else if (input1 == 7) return 7;
    else if (input1 == 8) return 8;
    else{
        cout << "Enter valid input! (1-8)" << endl;
        return -1;
    }

}