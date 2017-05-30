#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include <sys/unistd.h>
#include <signal.h>

#if (defined TARGET)
	#include <wiringPi.h>
#endif  

#define READ 0
#define WRITE 1

int decine = -1;
int fd_tens_in;
int fd_tens_out;

pid_t pidFiglio[7];
int fds[7][2];

const int segmenti[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0,},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};  //matrice che mappa ogni  segmento(riga) con ogni numero(colonna)
const int gpioTens[7]={3,12,30,14,13,0,2}; //mappa i pin gpio con i segmenti


void closeAll(){
	for(int i = 0; i < 7; i++){
		kill(pidFiglio[i], SIGKILL);
	}
	close(fd_tens_in);
	close (fd_tens_out);
	unlink("tens_pipe_out");
	exit(0);
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
	return atoi(buf);
}

void killHandler(int sig){
	closeAll();
}

void countHandler (int sig) {
	if(decine > 0){
		decine--;
		//printf("\nDecremento decine: %d\n", decine);
	}

	for(int i = 0; i < 7; i++){		
		char m[100];	
		sprintf(m, "n %d non non", decine);
		write(fds[i][WRITE], m, strlen(m) + 1);		

		#if (defined TARGET)
			//gipo decremento!
			wiringPiSetup () ;
	  		pinMode (gpioTens[i], OUTPUT);
	  		digitalWrite (gpioTens[i], !segmenti[decine][i]);
  		#endif

  	}

	if(decine == 0){
		kill(getExPid("units"), SIGUSR2);		
	}
}

void creazioneFigli(char ** argv){

	int pid;
	for(int i = 0; i<7; i++){
		pipe(fds[i]);
		pid = fork();
		pidFiglio[i] = pid;

		if (pid==0){
			unlink("tens_pipe_out");
			close(fd_tens_in);
			close (fd_tens_out);

			FILE *fd;

			int argv0size = strlen(argv[0]);
			strncpy(argv[0], "figliotens", argv0size);

			char messag[100];
			char comando[100];
			char tmpColor[50];

			char stato[50];
			char colore[50];
			strcpy(stato, "off");
			strcpy(colore, "red");

			while (1){							
				
				int bytesRead;
				while((bytesRead = read(fds[i][READ], messag, 100)) > 0){
					int valore=0;
					int led = 0;

					if(strncmp(messag, "n", 1) == 0){
						sscanf(messag, "n %d %s %s", &valore, comando, tmpColor);
						if(segmenti[valore][i] == 1){
							strcpy(stato, colore);
						}else{
							strcpy(stato, "off");
						}
						

						char directory[100];
						sprintf(directory, "../assets/tens_led_%d", i);
											
						
						fd=fopen(directory, "w");

						if(fd != NULL){
							fprintf(fd, "%s\n", stato);
  							fclose(fd);
						}

						if(strcmp(comando, "Info") == 0){
							printf("Stato LED Decine%d: %s \n",i, stato);
						}else if(strcmp(comando, "Color") == 0){
							strcpy(colore, tmpColor);
							printf("Colore settato: %s\n", colore);
						}						
					}
				}
			}

			exit(0);
		}	
	}

	//Handler per gestire la diminuzione delle decine
	void countHandler (int);
	signal (SIGUSR1, countHandler);

	//Handler per gestire la fine del countdown
	void killHandler (int);
	signal (SIGKILL, killHandler);
}

int readLine(int fd, char *str){
	int n; 
	
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}


int main(int argc, char ** argv){

	char decine_str[10];

	char str[100];
	char message[100];
	char readcolor[50];
	char msgPip[100];

	char richiesta[100];

	int led = -1;

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
			creazioneFigli(argv);
			if(decine == 0){
				kill(getExPid("units"), SIGUSR2);				
			}
		}else if(strcmp(message, "elapsed") == 0){
			sprintf(decine_str, "%d", decine);
			write (fd_tens_out, decine_str, strlen(decine_str) + 1);
		}else if(strcmp(message, "stop") == 0){
			closeAll();
		}else if(strcmp(message, "print") == 0){
			char a = 'a';
			for(int i = 0; i < 7; i++){
				a = 'a' + i;
				printf("%c: %d\n",a,segmenti[decine][i]);
			}
		}else if(strncmp(message, "info", 4) == 0){
			sscanf(message, "info %d", &led);
			sprintf(richiesta, "Info");
		}else if(strncmp(message, "color", 5) == 0){
			sscanf(message, "color %d %s", &led, readcolor);
			sprintf(richiesta, "Color %s", readcolor);
		}

		for(int i = 0; i < 7; i++){			
			if(led == i){
				sprintf(msgPip, "n %d %s", decine, richiesta);
				led = -1;
			}else{
				sprintf(msgPip, "n %d non non", decine);
			}
			write(fds[i][WRITE], msgPip, strlen(msgPip) + 1);
			
			#if (defined TARGET)
				//gipo !
				wiringPiSetup () ;
				pinMode (gpioTens[i], OUTPUT);
				digitalWrite (gpioTens[i], !segmenti[decine][i]);
			#endif
			
		}
	}
}

