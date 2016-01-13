#include <iostream>
#include <pthread.h>
#include "elevator.h"
#include "scheduler.h"
#include "log.h"

using namespace std;

#define NUM_ELEVATORS 1

pthread_mutex_t log_lock;

void run(Elevator& e) {
    while (1) {
        int floor = 0, dir = -1;
        displacement d;

        eprintf("\nEnter Floor number from 1-15: ");
        cin >> floor;
        if (floor < 1 || floor > MAX_FLOORS) {
            eprintf("floor out of range\n");
            continue;
        }

        eprintf("Up(1) or Down(0): ");
        cin >> dir;
        if (dir != 0 && dir != 1) {
            eprintf("\ninvalid direction\n");
            continue;
        }
        if (dir == 1)
            d = UP;
        else
            d = DOWN;

        schedule_job(e, floor, d);
    }
}

int main(int argc, char **argv) {

    Elevator elevator(1);
    pthread_mutex_init(&log_lock, NULL);

    // Start Processing Commands
    run(elevator);
    return 0;
}
