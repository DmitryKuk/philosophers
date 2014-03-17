To build:
	make

To run:
	./philosophers PHILOSOPHERS ALGORITHM EAT_TIMES
(where PHILOSOPHERS is number of philosophers,
ALGORITHM is 0 (the simpliest alrorithm with deadlocks)
or non-zero (algo with trywait, without deadlocks),
EAT_TIMES is times every philosopher need to eat)

To clear:
	make clear