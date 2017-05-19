	#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include <time.h>

int unita = 0;
int tensPid = 0;
int counttestPid = 0;

int getExPid(char* process){
	char comand[29];
	sprintf(comand, "pidof -s %s", process);

	FILE *ls = popen(comand, "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), ls) != 0) {
   	 	//printf("\n PID ( %s ) : %s", process, buf);
	}
	pclose(ls);
}

int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

void countHandler (int sig) { /* Executed if the child dies */
	printf("\nUNITS, countHandler\n");
	if (unita>0) unita--;
	printf("\nsecondi: %d\n", unita);
	if(unita == 0){
		printf("unità finite\n");
		kill(counttestPid,6);
		kill(tensPid, 16);
	}
	/*if (decine>=0) {
		if (decine!=0)decine--;
		kill(getExPid("units"), 18);
	}*/
	//printf("\nsegnale ricevuto,  secondi: %d",  unita);
}

int main(){
	tensPid=getExPid("tens");
	counttestPid=getExPid("counttest");
	//dichiaro l' handler per i segnali
	void countHandler (int);
	signal (16, countHandler);

	int fd_units_in;	
	int fd_units_out;

	char unita_str[10];

	char str[100];
	char message[100];

	do {
		fd_units_out = open ("units_pipe_in", O_WRONLY);
	} while (fd_units_out == -1);

	unlink("units_pipe_out");
	mknod ("units_pipe_out", S_IFIFO, 0);
	chmod ("units_pipe_out", 0660); 
	fd_units_in = open ("units_pipe_out", O_RDONLY);

	while (readLine (fd_units_in, str)) {
		sprintf(message, "%s", str);
			
		if(strncmp(message, "units", 5) == 0){
			sscanf(message, "units %d", &unita);
			write (fd_units_out, message, strlen(message) + 1);
			if(unita>0){
				char counttest[100];
				sprintf(counttest, "./counttest %d %d &", unita, getpid());
				printf("\n%s", counttest);
				system(counttest);
				//unita =0;
			}

		}else if(strcmp(message, "elapsed") == 0){
			sprintf(unita_str, "%d", unita);
			write (fd_units_out, unita_str, strlen(unita_str) + 1);
		}

	}

	close(fd_units_in);
	close (fd_units_out);
	unlink("units_pipe_out");
	
}

