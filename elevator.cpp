#include <iostream>
#include "elevator.h"
#include "log.h"


Elevator::Elevator(int idnum) {
    id = idnum;
    quit = 0;

    current_floor = 1;
    requested_floor = 1;
    next_floor = 1;
    direction_arrived = UP;
    elevator_state = INITIALIZING;

    pthread_cond_init(&elevator_wait, NULL);
    pthread_mutex_init(&elevator_mutex, NULL);

    pthread_create(&elevator_thread, NULL, elevator_run, this);
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
            Log::eprintf("%s:%s Waiting...\n", __func__, __FILE__);
            pthread_cond_wait(&me->elevator_wait, &me->elevator_mutex);
            if (me->quit == 1)
                return NULL;
            Log::eprintf("%s:%s Scheduer kicking in...\n", __func__, __FILE__);
        }
        me->elevator_state = RUNNING;

        Log::eprintf("%s:%s Elevator RUNNING dest %d\n", __func__, __FILE__, me->requested_floor);
        me->process_request();

        pthread_mutex_unlock(&me->elevator_mutex);
    }

    return NULL;
}

void Elevator::process_request() {
    next_floor = requested_floor;
    if (requested_floor > current_floor)
        direction_arrived = UP;
    else
        direction_arrived = DOWN;

    current_floor = next_floor;
}

bool Elevator::queue_next_request(int floor) {

    while (elevator_state != READY);
    requested_floor = floor;
    Log::eprintf("%s:%s Requested floor %d\n", __func__, __FILE__, requested_floor);

    return true;
}

bool Elevator::do_queue_pending_request(int floor, displacement d) {
    pthread_mutex_lock(&elevator_mutex);

    requests.push_back(make_pair(floor, d));
    Log::eprintf("%s:%s queue request floor:%d direction:%d sz %lu\n",
            __func__, __FILE__, floor, d, requests.size());
    pthread_cond_signal(&elevator_wait);

    pthread_mutex_unlock(&elevator_mutex);

    return true;
}

bool Elevator::queue_pending_request(int floor, displacement d) {
    pthread_mutex_lock(&elevator_mutex);

    requests.push_back(make_pair(floor, d));
    Log::eprintf("%s:%s queue request floor:%d direction:%d sz %lu\n",
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

    Log::eprintf("%s:%s Called... %lu\n", __func__, __FILE__, requests.size());
    if (requests.size() == 0) {
        Log::eprintf("%s:%s Queue is empty\n", __func__, __FILE__);
        return false;
    }

    if (requests.size() == 1) {
        queue_next_request(requests[0].first);
        Log::eprintf("%s:%s q size 1, next floor %d\n",
               __func__, __FILE__, requests[0].first);
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
            Log::eprintf("%s:%s cond 1 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_min_index].first);
            requests.erase(requests.begin() + same_d_min_index);
            return true;
        }

        if (opp_d_floor != -1) {
            queue_next_request(requests[opp_d_max_index].first);
            Log::eprintf("%s:%s cond 2 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_max_index].first);
            requests.erase(requests.begin() + opp_d_max_index);
            return true;
        }

        if (opp_d_low_floor != - 1) {
            queue_next_request(requests[opp_d_low_max_index].first);
            Log::eprintf("%s:%s cond 3 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_low_max_index].first);
            requests.erase(requests.begin() + opp_d_low_max_index);
            return true;
        }

        if (same_d_low_floor != MAX_FLOORS + 1) {
            queue_next_request(requests[same_d_low_min_index].first);
            Log::eprintf("%s:%s cond 4 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_low_min_index].first);
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
            Log::eprintf("%s:%s cond 5 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_max_index].first);
            requests.erase(requests.begin() + same_d_max_index);
            return true;
        }

        if (opp_d_floor != MAX_FLOORS + 1) {
            queue_next_request(requests[opp_d_min_index].first);
            Log::eprintf("%s:%s cond 6 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_min_index].first);
            requests.erase(requests.begin() + opp_d_min_index);
            return true;
        }

        if (opp_d_high_floor != -1) {
            queue_next_request(requests[opp_d_high_min_index].first);
            Log::eprintf("%s:%s cond 7 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[opp_d_high_min_index].first);
            requests.erase(requests.begin() + opp_d_high_min_index);
            return true;
        }

        if (same_d_high_floor != -1) {
            queue_next_request(requests[same_d_high_max_index].first);
            Log::eprintf("%s:%s cond 8 q size %d, next floor %d\n",
                __func__, __FILE__, size, requests[same_d_high_max_index].first);
            requests.erase(requests.begin() + same_d_high_max_index);
            return true;
        }
    }

    Log::eprintf("%s:%s Error! cannot process queue (%lu)\n",
            __func__, __FILE__, requests.size());
    return false;
}

int Elevator::get_current_floor() {
    pthread_mutex_lock(&elevator_mutex);

    while (elevator_state != READY);
    return current_floor;

    pthread_mutex_unlock(&elevator_mutex);
}

displacement Elevator::get_displacement() {
    pthread_mutex_lock(&elevator_mutex);

    while (elevator_state != READY);
    return direction_arrived;

    pthread_mutex_unlock(&elevator_mutex);
}
