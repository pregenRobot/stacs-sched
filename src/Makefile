schedulers:
	gcc -o sched -g ./fcfs.c ./p_rr.c ./rr.c ./sched.c
programs:
	gcc -o printchars -g ./printchars.c
all:
	gcc -D_GNU_SOURCE -o sched -g ./fcfs.c ./p_rr.c ./rr.c ./sched.c ./utility.c
	gcc -o printchars -g ./printchars.c
	gcc -o fail -g ./fail.c
clean:
	rm -f sched
	rm -f printchars
	rm -f fail