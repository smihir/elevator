#ifndef ELEVATOR_ELEVATOR_H
#define ELEVATOR_ELEVATOR_H

#include <pthread.h>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>

using namespace std;

#define MAX_FLOORS 15
#define MAX_BUFF 150

enum state {
    INITIALIZING,
    READY,
    RUNNING,
};

enum displacement {
    DOWN = 0,
    UP = 1,
};

class Elevator {
private:
    pthread_t elevator_thread;
    pthread_cond_t elevator_wait;
    pthread_mutex_t elevator_mutex;

    int id;
    int quit;
    int step;

    state elevator_state;

    int current_floor;
    displacement direction_arrived;
    pair<int, displacement> current_request;

    int requested_floor;
    int next_floor;
    vector< pair<int, displacement> > requests;

    static void *elevator_run(void *arg);
    void process_request();
    bool queue_next_request(int floor);
    bool schedule_next_request();
    void eprintf(const char *fmt, ...);

public:

    Elevator(int idnum);
    ~Elevator();
    bool do_queue_pending_request(int floor, displacement d);
    bool queue_pending_request(int floor, displacement d);
    bool do_pending_request();
    int get_current_floor();
    displacement get_displacement();
    state get_state();
    void get_full_state(int& floor, displacement& dir, state& st);
    void do_quit();
    void do_step();
};

#endif
