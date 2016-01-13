# Elevator
A Simple Elevator Simulator

## Compiling
Type the following command in the projects directory:
```
make
```

## Simulator
The elevator simulator consists of a scheduler which processes queued requests based on the current floor of the elevator
and the direction it took to arrive there. The scheduler will try to make sure that the next request leads it in the same direction, if
such a request is not found it can change its direction. For example:

```
Current floor is 4, and it reached the floor from floor 1. So the displacement for elevator was UP.

The current pending requests are:

No. Floor Direction
1      7     UP
2      9     UP
3      10    DOWN
4      11    UP

So the elevator algorithm will process requests in order 1, 2, 4, 3
```
This scheduler is better than the FCFS scheduler in a way that it makes sure that the elevator has to travel less when the requests are out of order and in real life situations when the elevator can be called on any random floor it will perform better than FCFS.

The algorithm in its current state can lead to starvation when a pending request makes the elevator change the direction, but newer requests are coming in (happens only in a small window when the scheduler has not run yet, but can be easily reproduced if the key presses are simulated by a machine) which cause the elevator go in same direction.
