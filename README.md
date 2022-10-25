# stacs-sched


## Building command 

```sh
cd src
make all
```
## Cleaning command

```sh
cd src
make clean
```

## Running the command

```sh
cd src
./sched ../configs/exec.conf fcfs ## Run the FCFS scheduler
```
```sh
cd src
./sched ../configs/exec.conf rr 100 ## Run the RR scheduler -- quantum = 100
```
```sh
cd src
./sched ../configs/exec.conf p_rr 100 ## Run the P_RR scheduler -- quantum = 100
```