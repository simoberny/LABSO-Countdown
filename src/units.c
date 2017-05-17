#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include <time.h>

int unita = 0;

void countdown(){
	while(unita >= 0){
		if(unita == 0){
			unita = 9;	
			// INVIO SEGNALE	
		}
		sleep(1);
		unita--;
	}

}

int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

int main(){
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
			countdown();
		}else if(strcmp(message, "elapsed") == 0){
			sprintf(unita_str, "%d", unita);
			write (fd_units_out, unita_str, strlen(unita_str) + 1);
		}

	}

	close(fd_units_in);
	close (fd_units_out);
	unlink("units_pipe_out");
	
}

