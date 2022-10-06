all:
	gcc -o sched -g ./*.c
clean:
	rm -f sched