#ifndef ELEVATOR_ELEVATOR_H
#define ELEVATOR_ELEVATOR_H

#include <pthread.h>

enum state {
    INITIALIZING,
    READY,
    RUNNING,
};

enum displacement {
    UP,
    DOWN,
};

class Elevator {
private:
    pthread_t elevator_thread;
    pthread_cond_t elevator_wait;
    pthread_mutex_t elevator_mutex;

    int id;
    state elevator_state;

    int current_floor;
    displacement direction_arrived;

    int requested_floor;
    int next_floor;

    void process_request();
public:

    Elevator(int idnum);
    bool queue_request(int floor);
    int get_current_floor();
    displacement get_displacement();
    static void *elevator_run(void *arg);
};

#endif
