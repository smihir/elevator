#include <iostream>
#include "elevator.h"


Elevator::Elevator(int idnum) {
    id = idnum;

    current_floor = 1;
    requested_floor = 1;
    next_floor = 1;
    direction_arrived = UP;
    elevator_state = INITIALIZING;

    pthread_cond_init(&elevator_wait, NULL);
    pthread_mutex_init(&elevator_mutex, NULL);

    pthread_create(&elevator_thread, NULL, elevator_run, this);
}

void *Elevator::elevator_run(void *arg) {
    Elevator *me = static_cast<Elevator *>(arg);

    if (me == NULL)
        return NULL;

    while (1) {
        pthread_mutex_lock(&me->elevator_mutex);
        me->elevator_state = READY;
        pthread_cond_wait(&me->elevator_wait, &me->elevator_mutex);
        me->elevator_state = RUNNING;

        me->process_request();

        pthread_mutex_unlock(&me->elevator_mutex);
    }

    return NULL;
}

void Elevator::process_request() {
    next_floor = requested_floor;
    sleep(5);
    if (requested_floor > current_floor)
        direction_arrived = UP;
    else
        direction_arrived = DOWN;

    current_floor = next_floor;
}

bool Elevator::queue_request(int floor) {
    pthread_mutex_lock(&elevator_mutex);

    while (elevator_state != READY);
    requested_floor = floor;
    pthread_cond_signal(&elevator_wait);

    pthread_mutex_unlock(&elevator_mutex);

    return true;
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
