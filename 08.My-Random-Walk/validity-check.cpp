int check1(int &input1){
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter a valid number input "<< endl;
        return 0;
    }
    if      (input1 == 1) return 1;
    else if (input1 == 2) return 2;
    else if (input1 == 3) return 3;
    else if (input1 == 4) return 4;
    else if (input1 == 5) return 5;
    else if (input1 == 6) return 6;
    else {
        cout << "Invalid Input ! Try again (1-6)" << endl;
        return -1;
    }
    
}
