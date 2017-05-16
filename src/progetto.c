#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "progetto.h"

int main(int argc, char *argv[]){
	char comando[20];
	pid_t pid_tens[7];
	pid_t pid_units[7];
	int secondi = 0;


	do{
		printf("Comandi disponibili: \n-start <secondi>: Avvio countdown\n-elapsed: Secondi rimasti\n-stop: Ferma il conto alla rovescia\n-tens: Decine\n-units: Unità\n-quit\n");
		printf("-----------------\n Run \n-----------------\n");
		scanf("%s", comando);

		if(strcmp(comando, "start")){
			printf("Conto alla rovescia di %d iniziato!", secondi);
		}
	}while(strcmp(comando, "quit"));
	return 0;
}
