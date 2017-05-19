#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include <time.h>

int unita = -1;
int decine = 1;

int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

void countHandler (int sig) {
	decine = 0;
}

int getExPid(char* process){
	char comand[29];
	sprintf(comand, "pidof -s %s", process);

	FILE *ls = popen(comand, "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), ls) != 0) {
   	 	//printf("\n PID ( %s ) : %s", process, buf);
	}
	pclose(ls);
	return buf;
}

int main(){
	void countHandler (int);
	signal (18, countHandler);

	int fd_units_in;
	int fd_units_out;
	clock_t start, end;
	int prevdiff = 0;
	char unita_str[10];

	char str[100];
	char message[100];

	do {
		fd_units_out = open ("units_pipe_in", O_WRONLY);
	} while (fd_units_out == -1);

	unlink("units_pipe_out");
	mknod ("units_pipe_out", S_IFIFO, 0);
	chmod ("units_pipe_out", 0660); 
	fd_units_in = open ("units_pipe_out", O_NONBLOCK);

	while(1){
		
		if (readLine (fd_units_in, str)) {
			sprintf(message, "%s", str);
			
			if(strncmp(message, "units", 5) == 0){
				sscanf(message, "units %d", &unita);
				write (fd_units_out, message, strlen(message) + 1);
				start = clock();
			}else if(strcmp(message, "elapsed") == 0){
				sprintf(unita_str, "%d", unita);
				write (fd_units_out, unita_str, strlen(unita_str) + 1);
			}
		}

		if(decine > 0){
			if(unita == 0){
				kill(getExPid("tens"), 17);
				unita = 10;
			}
		}else if(decine){
			unita = 10;
		}

		if(unita > 0){
			end = clock();
			int diff = ((end - start)/CLOCKS_PER_SEC);
			if((diff - prevdiff) >= 1){
				unita = unita - (diff - prevdiff);
				printf("Unità: %d\n", unita);
				prevdiff = diff;
			}
		}
	}


	close(fd_units_in);
	close (fd_units_out);
	unlink("units_pipe_out");
	
}

