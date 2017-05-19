#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include "progetto.h"

int fd_tens_in;
int fd_tens_out;

int fd_units_in;
int fd_units_out;


void creazionePipe(){
	//PIPE DI LETTURA DECINE
	unlink("tens_pipe_in");
	mknod ("tens_pipe_in", S_IFIFO, 0);
	chmod ("tens_pipe_in", 0660);
	fd_tens_in = open ("tens_pipe_in", O_NONBLOCK);

	//PIPE DI LETTURA UNITA'
	unlink("units_pipe_in");
	mknod ("units_pipe_in", S_IFIFO, 0);
	chmod ("units_pipe_in", 0660);
	fd_units_in = open ("units_pipe_in", O_NONBLOCK);
}

void closePipe(){
	close (fd_tens_out);
	close(fd_tens_in);
	close (fd_units_out);
	close(fd_units_in);
	unlink("tens_pipe_in");
	unlink("units_pipe_in");
}

int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}


void start(int sec){
	char strin_tens[100];
	char strin_units[100];	
	char strout_tens[100];
	char strout_units[100];

	int unita = sec%10;
	int decine = (sec)/10;

	sprintf(strout_tens, "tens %d", decine);
	sprintf(strout_units, "units %d", unita);

	system("./tens &");
	system("./units &");

	//Apertura Pipe di scrittura
	do {
		fd_tens_out = open ("tens_pipe_out", O_WRONLY);
	} while (fd_tens_out == -1);

	do {
		fd_units_out = open ("units_pipe_out", O_WRONLY);
	} while (fd_units_out == -1);

	write (fd_tens_out, strout_tens, strlen(strout_tens) + 1);
	write (fd_units_out, strout_units, strlen(strout_units) + 1);
}

void elapsed(){
	char decine[10];
	char unita[10];

	//Apertura Pipe di scrittura
	do {
		fd_tens_out = open ("tens_pipe_out", O_WRONLY);
	} while (fd_tens_out == -1);

	do {
		fd_units_out = open ("units_pipe_out", O_WRONLY);
	} while (fd_units_out == -1);
	
	write (fd_tens_out, "elapsed", strlen("elapsed") + 1);
	write (fd_units_out, "elapsed", strlen("elapsed") + 1);

	sleep(1);

	while(readLine(fd_tens_in, decine)){
	}

	while(readLine(fd_units_in, unita)){
	}

	printf("\n Elapsed time: %s%s \n", decine, unita);

	
}

int main(int argc, char *argv[]){
	char comando[20];
	int secondi = -1;

	creazionePipe();

	do{	
		printf("\nComandi disponibili: \n-start <secondi>: Avvio countdown\n-elapsed: Secondi rimasti\n-stop: Ferma il conto alla rovescia\n-tens: Decine\n-units: Unità\n-quit\n");
		printf("-----------------\n Run \n-----------------\n");
		if (fgets (comando , 20 , stdin) != NULL ){
			int l= strlen(comando);//togliamo l' invio che prende dentro in automatico
			comando[l-1] = '\0';
			
			if(strncmp(comando, "start", 5) == 0){
				sscanf(comando, "start %d", &secondi);
				if(secondi>-1&&secondi<60){
					printf("\nConto alla rovescia di %d iniziato!\n", secondi);
					start(secondi);
				}else{
					printf("\nInserire un tempo valido\n");				
				}
			}else if(strcmp(comando, "elapsed") == 0){
				elapsed();
			}else if(strcmp(comando, "stop") == 0){
				//TODO
			}else if(strcmp(comando, "tens") == 0){
				//TODO
			}else if(strcmp(comando, "unit") == 0){
				//TODO
			}else if(strcmp(comando, "quit") != 0){
				printf("\ncomando errato\n");
			}
		}
		
	}while(!(strcmp(comando, "quit") == 0));

	closePipe();

	return 0;
}
