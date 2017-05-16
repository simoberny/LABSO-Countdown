#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "progetto.h"

int main(int argc, char *argv[]){
	char comando[20];
	pid_t pid_tens[7];
	pid_t pid_units[7];
	int secondi = -1;


	do{
		printf("\nComandi disponibili: \n-start <secondi>: Avvio countdown\n-elapsed: Secondi rimasti\n-stop: Ferma il conto alla rovescia\n-tens: Decine\n-units: Unità\n-quit\n");
		printf("-----------------\n Run \n-----------------\n");
		//scanf("%s%d", comando, &secondi);
		if ( fgets (comando , 20 , stdin) != NULL ){
			sscanf(comando, "start %d", &secondi);
			if(secondi != -1){
				sscanf(comando, "%s ", comando);
			}
		}
		

		if(strcmp(comando, "start") == 0 && secondi != -1){
			printf("Conto alla rovescia di %d iniziato!", secondi);
		}else if(strcmp(comando, "quit") != 0 ){
			perror("Manca parametro o comando inesistente");
		}
	}while(strcmp(comando, "quit") == 0);
	return 0;
}
