#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>

int decine = -1;
int fd_tens_in;
int fd_tens_out;

int readLine(int fd, char *str){
	int n; 
	
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

void countHandler (int sig) {
	if(decine > 0){
		decine--;
		//printf("\nDecremento decine: %d\n", decine);
	}

	if(decine == 0){
		kill(getExPid("units"), 18);
		closeAll();
	}
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

void closeAll(){
	/*for(int i = 0; i < 7; i++){
		kill(pidFiglio[i], 9);
	}*/
	close(fd_tens_in);
	close (fd_tens_out);
	unlink("tens_pipe_out");
	exit(0);
}


int main(){
	void countHandler (int);
	signal (17, countHandler);

	char decine_str[10];

	char str[100];
	char message[100];

	do {
		fd_tens_out = open ("tens_pipe_in", O_WRONLY);
	} while (fd_tens_out == -1);

	unlink("tens_pipe_out");
	mknod ("tens_pipe_out", S_IFIFO, 0);
	chmod ("tens_pipe_out", 0660); 
	fd_tens_in = open ("tens_pipe_out", O_RDONLY);
	
	while(readLine (fd_tens_in, str)) {
		sprintf(message, "%s", str);
		if(strncmp(message, "tens", 4) == 0){
			sscanf(message, "tens %d", &decine);
			if(decine == 0){
				kill(getExPid("units"), 18);
				closeAll();
			}	// unità già nell' ultimo giro, decine gia finite (tutti i casi val <10)
		}else if(strcmp(message, "elapsed") == 0){
			sprintf(decine_str, "%d", decine);
			write (fd_tens_out, decine_str, strlen(decine_str) + 1);
		}else if(strcmp(message, "stop") == 0){
			closeAll();
		}else if(strcmp(message, "print") == 0){
			/*char a = 'a';
			for(int i = 0; i < 7; i++){
				a = 'a' + i;
				printf("%c: %d\n",a,segmenti[unita][i]);
			}*/
		}
	}
}

