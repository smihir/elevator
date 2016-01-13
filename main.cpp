#include <iostream>
#include <fstream>
#include <pthread.h>
#include <sstream>
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

    if (argc == 2) {
        ifstream tfile;
        tfile.open(argv[1]);
        string line;

        if (tfile.is_open()) {
            int floor = 1;
            int dir = 0;
            displacement d;
            while (getline(tfile, line)) {
                istringstream iss(line);
                if (dir == 0)
                    d = DOWN;
                else
                    d = UP;
                iss >> floor;
                cout << "floor: " << floor << " dir: " << dir << endl;
                elevator.queue_pending_request(floor, d);
            }
            elevator.do_pending_request();
            sleep(15);
            elevator.do_quit();
            return 0;
        } else {
            cout << "Cannot open file" << endl;
            return 1;
        }

    }

    // Start Processing Commands
    run(elevator);
    return 0;
}
