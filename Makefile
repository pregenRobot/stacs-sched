schedulers:
	gcc -o sched -g ./fcfs.c ./mlfq.c ./rr.c ./sched.c
programs:
	gcc -o printchars -g ./printchars.c
all:
	gcc -D_GNU_SOURCE -o sched -g ./fcfs.c ./mlfq.c ./rr.c ./sched.c ./utility.c
	gcc -o printchars -g ./printchars.c
	gcc -o fail -g ./fail.c
clean:
	rm -f sched