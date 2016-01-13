CC = g++
CSCOPE = cscope
CFLAGS += -Wall -Werror
LDFLAGS += -lpthread

ELEVATOR-OBJS := main.o scheduler.o elevator.o\

ifeq ($(DEBUG), y)
 CFLAGS += -g -DDEBUG
endif

.PHONY: all
all: elevator

elevator: $(ELEVATOR-OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(ELEVATOR-OBJS) -o $@

%.o: %.c *.h
	$(CC) $(CFLAGS) -o $@ -c $<

cscope:
	$(CSCOPE) -bqR

.PHONY: clean
clean:
	rm -rf *.o elevator
