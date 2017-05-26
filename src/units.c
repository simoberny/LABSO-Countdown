#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
//#include <time.h>

#define READ 0
#define WRITE 1

int unita = -1;
int decine = 1;
int fd_units_in;
int fd_units_out;

pid_t pidFiglio[7];
int fds[7][2];

const int segmenti[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0,},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};  //matrice che mappa ogni  segmento(riga) con ogni numero(colonna)

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

void countHandler (int sig) { // Funzione che gestisce il segnale che manda le decine quando sono finite
	decine = 0; // Permette alle unità di eseguire un ultimo ciclo;
	sleep(1);
}

void creazioneFigli(char ** argv){

int pid;
	for(int i = 0; i<7; i++){
		pipe(fds[i]);
		pid = fork();
		pidFiglio[i] = pid;

		if (pid==0){
			unlink("tens_pipe_out");
			close(fd_units_in);
			close (fd_units_out);

			int argv0size = strlen(argv[0]);
			strncpy(argv[0], "figliounits", argv0size);
			char messag[100];

			char stato[50];
			char colore[50];
			strcpy(stato, "off");
			strcpy(colore, "red");

			while (1){
				int bytesRead = read(fds[i][READ], messag, 100);

				int valore=0;
				int led = 0;

				if(strncmp(messag, "Numero", 6) == 0){
					sscanf(messag, "Numero: %d", &valore);
					if(segmenti[valore][i] == 1){
						strcpy(stato, colore);
					}else{
						strcpy(stato, "off");
					}
					//printf("Numero: %d - Stato: %s\n", valore, stato);
				}else if(strncmp(messag, "Info", 4) == 0){
					sscanf(messag, "Info %d", &led);
					printf("Stato LED %d: %s \n", led, stato);
				}else if(strncmp(messag, "Color", 5) == 0){
					sscanf(messag, "Color %s", &colore);
					printf("Colore settato: %s\n", colore);
				}

			}

			exit(0);
		}	
	}

	void countHandler (int);
	signal (18, countHandler);
}

int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

void closeAll(){
	for(int i = 0; i < 7; i++){
		kill(pidFiglio[i], 9);
	}
	close(fd_units_in);
	close (fd_units_out);
	unlink("units_pipe_out");
	exit(0);
}

int main(int argc, char ** argv){
	char unita_str[10];

	char str[100];
	char message[100];
	char readcolor[50];

	clock_t start, end;
	int prevdiff = 0;
	int led = 0;

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
				start = clock();
				creazioneFigli(argv);
			}else if(strcmp(message, "elapsed") == 0){
				sprintf(unita_str, "%d", unita);
				write (fd_units_out, unita_str, strlen(unita_str) + 1);
			}else if(strcmp(message, "stop") == 0){
				closeAll();
			}else if(strcmp(message, "print") == 0){
				char a = 'a';
				for(int i = 0; i < 7; i++){
					a = 'a' + i;
					printf("%c: %d\n",a,segmenti[unita][i]);
				}
			}else if(strncmp(message, "info", 4) == 0){
				sscanf(message, "info %d", &led);
				char prova[100];
				sprintf(prova, "Info %d", led);
				write(fds[led][WRITE], prova, strlen(prova)+1);

			}else if(strncmp(message, "color", 5) == 0){
				sscanf(message, "color %d %s", &led, readcolor);
				char prova[100];
				sprintf(prova, "Color %s", readcolor);
				write(fds[led][WRITE], prova, strlen(prova)+1);
			}
		}

		if(unita > 0){ //Se ci sono unità le decremento 

			sleep(1);
			unita -= 1;
			
			//printf("Unità: %d\n", unita);

			//Scrivo nelle pipe anonime del figlio

			char msgPip[100];
			sprintf(msgPip, "Numero: %d", unita);

			for(int i = 0; i < 7; i++){
				write(fds[i][WRITE], msgPip, strlen(msgPip) + 1);
			}
		}

		if(decine > 0){			
			if(unita == 0){ // Se le decine sono > 0 e sono arrivato a 0 con le unità invio segnale alle decine per decrementarsi
				kill(getExPid("tens"), 17);
				unita = 10; // E aggiorno le unità per ripartire
			}
		}

		if(decine == 0 && unita == 0){
			printf("timer completato\n");
			//printf("pid padre: %d   PID DA getExPid: %d\n", getpid(), getExPid("units"));
			closeAll();
		}
	}
}

