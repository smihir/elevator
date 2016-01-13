#include <iostream>
#include "scheduler.h"


void schedule_job(Elevator& elevator, int floor, displacement direction) {
    printf("\n%s:%s queue request floor:%d direction:%d\n",
            __func__, __FILE__, floor, direction);
    elevator.queue_pending_request(floor, direction);
}
