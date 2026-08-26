#include <iostream>
#include <limit>
#include <random>
#include "validity-check.h"
#include "test.h"
using namespace std;

int main(){
    int input1
    

    while(true){
        cout << endl;
        cout << "Random Walk Simulation" << endl;
        cout << "1. 1 Particle" << endl;
        cout << "2. 5 Particle" << endl;
        cout << "3. 10 Particle" << endl;
        cout << "4. custome " << endl;
        cout << "5. Default " << endl;
        cout << "6. Back" << endl;
        cout << "Choose your choice : " << endl;
        
        cin >> input1; 
        int choose1 = check1(input1);

        if (choose1 == 6){
            break;
        }else if(choose1 == 1){
            while(true){
                test1();
            }
        }
    }
}