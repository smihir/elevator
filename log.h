#ifndef ELEVATOR_LOG_H
#define ELEVATOR_LOG_H

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

class Log {
public:
    static pthread_mutex_t log_lock;
    static inline void eprintf(const char *fmt, ...) {
        pthread_mutex_lock(&log_lock);

        char logbuff[MAX_BUFF] = {'\0'};
        va_list args;
        va_start(args, fmt);
        vsprintf(logbuff, fmt, args);
        //cout << logbuff << flush;
        printf("%s", logbuff);

        pthread_mutex_unlock(&log_lock);
    }
};
#endif
