// starting with the pie estimate, as i mentioned earlier in chater 1 that i will make it 
// for better understanding and analysis 

#include <iostream>
#include <random>
using namespace std;
#include "validity-check.cpp"

int main(){
    int input1;
    while(true){
        cout << endl;
        cout << "Monte Carlo Pie Estimate :- " << endl;
        cout << "1. Existing Tests." << endl;
        cout << "2. 10 Tests."<< endl;
        cout << "3. 100 Tests." << endl;
        cout << "4. 1000 Tests." << endl;
        cout << "5. 10000 Tests." << endl;
        cout << "6. 100000 Tests." << endl;
        cout << "7. Custome Tests." << endl;
        cout << "8. Back." << endl;
        cout << endl;
        cout << "Enter the serial number of the test you want to perform : " << endl;

        cin >> input1;
        int choice1 = check1(input1);

        if (choice1 == 8){
            break;
        }else if(choice1 == 1){
            
        }
    }

}