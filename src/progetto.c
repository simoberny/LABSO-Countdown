#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "progetto.h"

pid_t units;
pid_t tens;
int fd;

void countdown(int sec){
	char message[100];

	sprintf (message,"Secondi: %d", sec);
		system("./tens &");

		do {
			fd = open ("tens_pipe", O_WRONLY);
		} while (fd == -1);

		write (fd, message, strlen(message) + 1);
		close (fd);
		printf("Terminating child");
}

int main(int argc, char *argv[]){
	char comando[20];
	int secondi = -1;

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
					countdown(secondi);
				}else{
					printf("\nInserire un tempo valido\n");				
				}
			}else if(strcmp(comando, "elapsed") == 0){
				printf("\nelapsed time\n");	
				//TODO
			}else if(strcmp(comando, "stop") == 0){
				//TODO
			}else if(strcmp(comando, "tens") == 0){
				//TODO
			}else if(strcmp(comando, "unit") == 0){
				//TODO
			}else{
				printf("\ncomando errato\n");
			}
		}
		
	}while(!(strcmp(comando, "quit") == 0));

	return 0;
}
