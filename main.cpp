#include <iostream>
#include <pthread.h>
#include "elevator.h"
#include "scheduler.h"

using namespace std;

#define NUM_ELEVATORS 1
#define MAX_FLOORS 15

void run(void) {
    while (1) {
        int floor = 0, dir = -1;
        cout << "Enter Floor number from 1-15: ";
        cin >> floor;
        if (floor < 1 || floor > 15) continue;

        cout << "Up(1) or Down(0): ";
        cin >> dir;
        if (dir != 0 || dir != 1) continue;

        schedule(1, floor, dir);
    }
}

int main(int argc, char **argv) {

    Elevator elevator(1);

    // Start Processing Commands
    run();
    return 0;
}
