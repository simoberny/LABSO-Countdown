#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "progetto.h"

pid_t units;
pid_t tens;
int fd;

void countdown(int sec){
	units = fork();
	tens = fork();
	unlink("units_pipe");
	unlink("tens_pipe");
	mknod("units_pipe", S_IFIFO);

	if(units == 0){

	}else if(tens == 0){
		
	}
}

int main(int argc, char *argv[]){
	char comando[20];
	int secondi = -1;

	do{
		printf("\nComandi disponibili: \n-start <secondi>: Avvio countdown\n-elapsed: Secondi rimasti\n-stop: Ferma il conto alla rovescia\n-tens: Decine\n-units: Unità\n-quit\n");
		printf("-----------------\n Run \n-----------------\n");
		if ( fgets (comando , 20 , stdin) != NULL ){
			sscanf(comando, "start %d", &secondi);
			if(secondi != -1){
				sscanf(comando, "%s ", comando);
			}
		}

		if(strcmp(comando, "start") == 0 && secondi != -1){
			countdown(secondi);
			printf("\nConto alla rovescia di %d iniziato!\n", secondi);
		}else if(strcmp(comando, "quit") != 0 ){
			perror("Manca parametro o comando inesistente");
		}
	}while(strcmp(comando, "quit") == 0);
	return 0;
}
