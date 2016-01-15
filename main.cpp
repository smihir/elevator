#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "elevator.h"
#include "scheduler.h"
#include "log.h"

using namespace std;

#define MAX_ELEVATORS 4

pthread_mutex_t Log::log_lock;

void run(vector<Elevator *>& e) {
    while (1) {
        int floor = 0, dir = -1;
        displacement d;

        Log::eprintf("\nEnter Floor number from 1-15: \n");
        cin >> floor;
        if (floor < 1 || floor > MAX_FLOORS) {
            Log::eprintf("floor out of range\n");
            continue;
        }

        Log::eprintf("Up(1) or Down(0): \n");
        cin >> dir;
        if (dir != 0 && dir != 1) {
            Log::eprintf("\ninvalid direction\n");
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

    vector<Elevator *> elevators;

    for (int i = 0; i < MAX_ELEVATORS; i++)
        elevators.push_back(new Elevator(i));

    pthread_mutex_init(&Log::log_lock, NULL);

    if (argc == 2) {
        ifstream tfile;
        tfile.open(argv[1]);
        string line;

        if (tfile.is_open()) {
            int floor = 1;
            int dir = 0;
            int id = 0;
            displacement d;
            while (getline(tfile, line)) {
                istringstream iss(line);
                iss >> id;
                iss >> floor;
                iss >> dir;
                d = (dir == 0) ? DOWN : UP;
                if (id >= MAX_ELEVATORS) {
                    continue;
                }
                //cout << "id: " << id << " floor: " << floor << " dir: " << dir << endl;
                //elevators[id]->queue_pending_request(floor, d);
                schedule_job(elevators, floor, d);
            }
            for (int i = 0; i < MAX_ELEVATORS; i++)
                elevators[i]->do_pending_request();
            sleep(5);
            for (int i = 0; i < MAX_ELEVATORS; i++)
                elevators[i]->do_quit();
            return 0;
        } else {
            cout << "Cannot open file" << endl;
            return 1;
        }

    }

    // Manual simulation, print info about each step
    for (auto e : elevators) {
        e->do_step();
    }

    // Start Processing Commands
    run(elevators);
    return 0;
}
