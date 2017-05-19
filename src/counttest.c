#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>


int main(int argc, char* argv[]){
	//countDown(argv[1]);
	
	pid_t pid;
	sscanf(argv[2], "%u", &pid);

	if (argv[1]!=0 && argc == 3){
		for(int i = 0; i < atoi(argv[1]); i++){
			sleep(1);
			kill(pid, 16);
		}
		printf("contatore finito (sec: %s)\n", argv[1]);
	}else{
		printf("contatore rifiutato\n");
	}
	
	return(0);
}

