# Elevator
A Simple Elevator Simulator

## Compiling
Type the following command in the projects directory:
```
make
```

## Simulator
The idea is to write a simulator for a group of elevators which can serve all the floors in a building. One thing which is to be
noted is that when a person outside the elevator presses a key to go UP or DOWN any of the elevators can be used to service this
request. But when a person is inside an elevator and wants to go to a particular floor then only that elevator can serve him.
For this purpose I have written 2 schdulers; one scheduler schedules the request for a particular elevator the other scheduler
will dispatch the request to one of the elevators after quering their status.

### Per-Elevator Scheduler
The elevator scheduler processes queued requests for that particular elevator. At every floor it re-analyzes it queue to choose the
best request. This is important because say that someone is in the elevator going from 4th to 15th floor and when the elevator is still on 6th floor you press the button to go up from 8th floor, then the elevator should stop for you at the 8th floor. The scheduler will try to choose the next request so that sure that it leads in the same direction the elevator is going, if
such a request is not found it can change its direction (thing of the SCAN algorithm in hard disks). For example:

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

### Global Scheduler
The global scheduler will recieve the requests sent from outside the elevator and dispatch it to the most favourable elevator. The most favourable elevator is chosen using a simple formula based on the elevators direction, current floor and whether the elevator is stationary or not. An elevator which is moving towards the target and has the least distance is the most preferred, the next preference is to a stationary elevator which is nearer to the target and the least preferred once are the elevators which are moving away from the target and are nearest to the target.
