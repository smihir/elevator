#include <iostream>
#include <atomic>
#include <limits.h>
#include "scheduler.h"
#include "elevator.h"
#include "log.h"

int score(int e_floor, displacement e_dir, state e_state,
          int floor) {
    int d = floor - e_floor;

    if (e_state != READY) {
        if (d > 0) {
            if (e_dir == UP) {
                // The idea is to give more priority
                // to the elevator which is moving
                return 2*(MAX_FLOORS - d);
            } else {
                // elevator going away so elevator
                // near MAX_FLOOR or 1 floor will
                // have highest priority. do -1
                // to handle corner case when
                // d = MAX_FLOORS for both cases
                return d - MAX_FLOORS - 1;
            }
        } else {
            if (e_dir == DOWN) {
                return MAX_FLOORS - abs(d);
            } else {
                // elevator going away so elevator
                // near MAX_FLOOR or 1 floor will
                // have highest priority. do -1
                // to handle corner case when
                // d = MAX_FLOORS for both cases
                return abs(d) - MAX_FLOORS - 1;
            }
        }
    }

    // elevator is not moving
    return MAX_FLOORS - abs(d);
}

void schedule_job(vector<Elevator *>& elevator, int floor, displacement direction) {
    Log::eprintf("\n%s:%s queue request floor:%d direction:%d\n",
            __func__, __FILE__, floor, direction);

    Elevator *sched_e = nullptr;
    int max_score = INT_MIN;

    Log::eprintf("%s:%s Print Scores\n", __func__, __FILE__);
    for (auto e : elevator) {
        int e_floor;
        displacement e_dir;
        state e_state;
        int sc;

        e->get_full_state(e_floor, e_dir, e_state);
        sc = score(e_floor, e_dir, e_state, floor);
        Log::eprintf("%s:%s Scores %d\n", __func__, __FILE__, sc);
        if (sc > max_score) {
            max_score = sc;
            sched_e = e;
        } else if (sc == max_score) {
            // break tie based on direction the passenger
            // wants to go in
            if (direction == e_dir)
                sched_e = e;
        }
    }
    sched_e->do_queue_pending_request(floor, direction);
}
