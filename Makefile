CC = g++
CSCOPE = cscope
CFLAGS = -Wall -Werror -std=c++11
LDFLAGS = -lpthread

ELEVATOR-OBJS := main.o scheduler.o elevator.o\

ifeq ($(DEBUG), y)
	CFLAGS += -g -DDEBUG
endif

.PHONY: all
all: elevator

%.o: %.cpp *.h
	$(CC) $(CFLAGS) -std=c++11 -o $@ -c $<

elevator: $(ELEVATOR-OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(ELEVATOR-OBJS) -o $@

cscope:
	$(CSCOPE) -bqR

.PHONY: clean
clean:
	rm -rf *.o elevator
