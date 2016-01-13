# Elevator V1
The version 1 of the elevator consists of a scheduler which processes queued requests based on the current floor of the elevator
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
