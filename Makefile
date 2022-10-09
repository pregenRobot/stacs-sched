schedulers:
	gcc -o sched -g ./fifo.c ./pq.c ./rr.c ./sched.c
programs:
	gcc -o printchars -g ./printchars.c
all:
	gcc -o sched -g ./fifo.c ./pq.c ./rr.c ./sched.c
	gcc -o printchars -g ./printchars.c
clean:
	rm -f sched