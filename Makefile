fremen: fremen.c cmd.c utilities.c connectivity.c
	gcc -std=gnu11 -Wall -Wextra -lpthread fremen.c -o fremen.exe cmd.c utilities.c connectivity.c

atreides: Atreides.c cmd.c utilities.c connectivity.c
	gcc Atreides.c -o atreides.exe cmd.c utilities.c connectivity.c -Wall -Wextra -lpthread

install:
	fremen.exe config.txt
