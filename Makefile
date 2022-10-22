schedulers:
	gcc -o sched -g ./fifo.c ./mlfq.c ./rr.c ./sched.c
programs:
	gcc -o printchars -g ./printchars.c
all:
	gcc -D_GNU_SOURCE -o sched -g ./fifo.c ./mlfq.c ./rr.c ./sched.c ./timer.c
	gcc -o printchars -g ./printchars.c
clean:
	rm -f sched