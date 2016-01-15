#include <iostream>
#include <unistd.h>
#include "elevator.h"
#include "log.h"

Elevator::Elevator(int idnum) {
    id = idnum;
    quit = 0;
    step = 0;
    current_floor = 1;
    requested_floor = 1;
    next_floor = 1;
    direction_arrived = UP;
    elevator_state = INITIALIZING;
    current_request = make_pair(-1, DOWN);

    pthread_cond_init(&elevator_wait, NULL);
    pthread_mutex_init(&elevator_mutex, NULL);

    pthread_create(&elevator_thread, NULL, elevator_run, this);
}

void Elevator::eprintf(const char *fmt, ...) {

    char logbuff[MAX_BUFF];
    va_list args;
    va_start(args, fmt);
    vsprintf(logbuff, fmt, args);
    Log::eprintf("[id: %d] %s", id, logbuff);
}

Elevator::~Elevator() {
    do_quit();
    requests.erase(requests.begin(), requests.end());
}

void Elevator::do_quit() {
    quit = 1;
    pthread_cond_signal(&elevator_wait);
    pthread_join(elevator_thread, NULL);
}

void *Elevator::elevator_run(void *arg) {
    Elevator *me = static_cast<Elevator *>(arg);

    if (me == NULL)
        return NULL;

    while (me->quit == 0) {
        pthread_mutex_lock(&me->elevator_mutex);
        me->elevator_state = READY;
        while (me->schedule_next_request() == false) {
            me->eprintf("%s:%s Waiting...\n", __func__, __FILE__);
            pthread_cond_wait(&me->elevator_wait, &me->elevator_mutex);
            if (me->quit == 1)
                return NULL;
            me->eprintf("%s:%s Scheduer kicking in...\n", __func__, __FILE__);
        }
        me->elevator_state = RUNNING;

        me->eprintf("%s:%s Elevator RUNNING dest %d\n", __func__, __FILE__, me->requested_floor);
        pthread_mutex_unlock(&me->elevator_mutex);

        me->process_request();

    }

    return NULL;
}

void Elevator::process_request() {
    pthread_mutex_lock(&elevator_mutex);
    if (requested_floor > current_floor) {
        direction_arrived = UP;
        // a granular movement of 1 floor after
        // which the elevator will re-evaluate
        // all the requests
        next_floor = current_floor + 1;
    } else {
        direction_arrived = DOWN;
        // a granular movement of 1 floor after
        // which the elevator will re-evaluate
        // all the requests
        next_floor = current_floor - 1;
    }
    pthread_mutex_unlock(&elevator_mutex);
    //
    // This Just represents the transition of elevator
    // from one floor to next
    if (step == 1) {
        sleep(5);
    }

    pthread_mutex_lock(&elevator_mutex);
    current_floor = next_floor;
    eprintf("%s:%s On floor %d\n", __func__, __FILE__, 
                    current_floor);
    if (current_floor == current_request.first) {
        // Current request is completed, invalidate it
        current_request.first = -1;
        eprintf("%s:%s Done request for floor %d\n", __func__, __FILE__, 
                      requested_floor);
    }
    pthread_mutex_unlock(&elevator_mutex);
}

bool Elevator::queue_next_request(int floor) {

    while (elevator_state != READY);
    requested_floor = floor;
    eprintf("%s:%s Requested floor %d\n", __func__, __FILE__, requested_floor);

    return true;
}

bool Elevator::do_queue_pending_request(int floor, displacement d) {
    pthread_mutex_lock(&elevator_mutex);

    requests.push_back(make_pair(floor, d));
    eprintf("%s:%s queue request floor:%d direction:%d sz %lu\n",
            __func__, __FILE__, floor, d, requests.size());
    pthread_cond_signal(&elevator_wait);

    pthread_mutex_unlock(&elevator_mutex);

    return true;
}

bool Elevator::queue_pending_request(int floor, displacement d) {
    pthread_mutex_lock(&elevator_mutex);

    requests.push_back(make_pair(floor, d));
    eprintf("%s:%s queue request floor:%d direction:%d sz %lu\n",
            __func__, __FILE__, floor, d, requests.size());
    pthread_mutex_unlock(&elevator_mutex);

    return true;
}

bool Elevator::do_pending_request() {
    pthread_mutex_lock(&elevator_mutex);
    
    pthread_cond_signal(&elevator_wait);
    
    pthread_mutex_unlock(&elevator_mutex);
    
    return true;
}

bool Elevator::schedule_next_request() {

    if (requests.size() == 0 && current_request.first == -1) {
        eprintf("%s:%s Queue is empty\n", __func__, __FILE__);
        return false;
    }

    // We will have to re-evaluate the current request if it is valid.
    // This is because a new request can come in even even the elevator
    // is processing a current request
    if (current_request.first != -1)
        requests.push_back(current_request);

    if (requests.size() == 1) {
        queue_next_request(requests[0].first);
        eprintf("%s:%s q size 1, next floor %d\n",
               __func__, __FILE__, requests[0].first);
        current_request = requests[0];
        requests.erase(requests.begin());
        return true;
    }

    int size = requests.size();

    if (direction_arrived == UP) {
        int same_d_floor = MAX_FLOORS + 1, same_d_min_index;
        int opp_d_floor = -1, opp_d_max_index;

        int same_d_low_floor = MAX_FLOORS + 1, same_d_low_min_index;
        int opp_d_low_floor = -1, opp_d_low_max_index;

        pair<int, displacement> p;

        for (int i = 0; i < size; i++) {
            p = requests[i];
            if (p.first > current_floor) {
                if (p.second == direction_arrived) {
                    same_d_min_index = p.first < same_d_floor ? i : same_d_min_index;
                    same_d_floor = min(same_d_floor, p.first);
                } else {
                    opp_d_max_index = p.first > opp_d_floor ? i : opp_d_max_index;
                    opp_d_floor = max(opp_d_floor, p.first);
                }
            } else {
                if (p.second != direction_arrived) {
                    opp_d_low_max_index = p.first > opp_d_low_floor ? i : opp_d_low_max_index;
                    opp_d_low_floor = max(opp_d_low_floor, p.first);
                } else {
                    same_d_low_min_index = p.first < same_d_floor ? i : same_d_low_min_index;
                    same_d_low_floor = min(same_d_low_floor, p.first);
                }
            }
        }

        if (same_d_floor != MAX_FLOORS + 1) {
            queue_next_request(requests[same_d_min_index].first);
            eprintf("%s:%s cond 1 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_min_index].first);
            current_request = requests[same_d_min_index];
            requests.erase(requests.begin() + same_d_min_index);
            return true;
        }

        if (opp_d_floor != -1) {
            queue_next_request(requests[opp_d_max_index].first);
            eprintf("%s:%s cond 2 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_max_index].first);
            current_request = requests[opp_d_max_index];
            requests.erase(requests.begin() + opp_d_max_index);
            return true;
        }

        if (opp_d_low_floor != - 1) {
            queue_next_request(requests[opp_d_low_max_index].first);
            eprintf("%s:%s cond 3 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_low_max_index].first);
            current_request = requests[opp_d_low_max_index];
            requests.erase(requests.begin() + opp_d_low_max_index);
            return true;
        }

        if (same_d_low_floor != MAX_FLOORS + 1) {
            queue_next_request(requests[same_d_low_min_index].first);
            eprintf("%s:%s cond 4 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_low_min_index].first);
            current_request = requests[same_d_low_min_index];
            requests.erase(requests.begin() + same_d_low_min_index);
            return true;
        }

    } else {
        int same_d_floor = -1, same_d_max_index;
        int opp_d_floor = MAX_FLOORS + 1, opp_d_min_index;

        int same_d_high_floor = -1, same_d_high_max_index;
        int opp_d_high_floor = MAX_FLOORS + 1, opp_d_high_min_index;

        pair<int, displacement> p;

        for (int i = 0; i < size; i++) {
            p = requests[i];
            if (p.first < current_floor) {
                if (p.second == direction_arrived) {
                    same_d_max_index = p.first > same_d_floor ? i : same_d_max_index;
                    same_d_floor = max(same_d_floor, p.first);
                } else {
                    opp_d_min_index = p.first < opp_d_floor ? i : opp_d_min_index;
                    opp_d_floor = min(opp_d_floor, p.first);
                }
            } else {
                if (p.second != direction_arrived) {
                    opp_d_high_min_index = p.first < opp_d_high_floor ? i : opp_d_high_min_index;
                    opp_d_high_floor = min(opp_d_high_floor, p.first);
                } else {
                    same_d_high_max_index = p.first > same_d_high_floor ? i : same_d_high_max_index;
                    same_d_high_floor = max(same_d_high_floor, p.first);
                }
            }
        }

        if (same_d_floor != -1) {
            queue_next_request(requests[same_d_max_index].first);
            eprintf("%s:%s cond 5 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_max_index].first);
            current_request = requests[same_d_max_index];
            requests.erase(requests.begin() + same_d_max_index);
            return true;
        }

        if (opp_d_floor != MAX_FLOORS + 1) {
            queue_next_request(requests[opp_d_min_index].first);
            eprintf("%s:%s cond 6 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_min_index].first);
            current_request = requests[opp_d_min_index];
            requests.erase(requests.begin() + opp_d_min_index);
            return true;
        }

        if (opp_d_high_floor != -1) {
            queue_next_request(requests[opp_d_high_min_index].first);
            eprintf("%s:%s cond 7 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_high_min_index].first);
            current_request = requests[opp_d_high_min_index];
            requests.erase(requests.begin() + opp_d_high_min_index);
            return true;
        }

        if (same_d_high_floor != -1) {
            queue_next_request(requests[same_d_high_max_index].first);
            eprintf("%s:%s cond 8 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_high_max_index].first);
            current_request = requests[same_d_high_max_index];
            requests.erase(requests.begin() + same_d_high_max_index);
            return true;
        }
    }

    eprintf("%s:%s Error! cannot process queue (%lu)\n",
            __func__, __FILE__, requests.size());
    return false;
}

int Elevator::get_current_floor() {
    pthread_mutex_lock(&elevator_mutex);

    return current_floor;

    pthread_mutex_unlock(&elevator_mutex);
}

state Elevator::get_state() {
    pthread_mutex_lock(&elevator_mutex);

    return elevator_state;

    pthread_mutex_unlock(&elevator_mutex);
}

displacement Elevator::get_displacement() {
    pthread_mutex_lock(&elevator_mutex);

    return direction_arrived;

    pthread_mutex_unlock(&elevator_mutex);
}

void Elevator::get_full_state(int& floor, displacement& dir, state& st) {
    pthread_mutex_lock(&elevator_mutex);

    floor = current_floor;
    st = elevator_state; 
    dir = direction_arrived;

    pthread_mutex_unlock(&elevator_mutex);
}

void Elevator::do_step() {
    step = 1;
}
