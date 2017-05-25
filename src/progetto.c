#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include "progetto.h"

#define C(x) ((_[*n-'0'])>>s)&m&x
#define P putchar

int fd_tens_in;
int fd_tens_out;

int fd_units_in;
int fd_units_out;

unsigned _[]={476,144,372,436,184,428,492,148,508,188};

void p(int a,char*n,unsigned m,int s){
	for(;isdigit(*n);++n){
		P(C(1)?'|':' ');
		for(int i=0;i<a;++i){
			P(C(4)?'_':' ');
		}
		P(C(2)?'|':' ');
	}
		P('\n');
}
void l(int a,char*b){
	p(a,b,7,0);int i=1;
	for(;i<a;++i)p(a,b,3,3);p(a,b,7,3);i=1;
	for(;i<a;++i)p(a,b,3,6);p(a,b,7,6);
}

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

	//printf("PID: %d\n", getExPid("tens"));

	if(getExPid("tens") != 0){
		write (fd_tens_out, "elapsed", strlen("elapsed") + 1);

		while(!readLine(fd_tens_in, decine)){
		}

	}else{
		strcpy(decine, "0");
	}

	if(getExPid("units") != 0){
		write (fd_units_out, "elapsed", strlen("elapsed") + 1);

		while(!readLine(fd_units_in, unita)){
		}
	}else{
		strcpy(unita, "0");
	}


	char numero[10];
	sprintf(numero, "%s%s", decine, unita);
	l(3, numero);
}

void stop(){
	if(getExPid("tens") != 0){
		write (fd_tens_out, "stop", strlen("stop") + 1);
	}

	if(getExPid("units") != 0){
		write (fd_units_out, "stop", strlen("stop") + 1);
	}
}

void printTens(){
	if(getExPid("tens") != 0){
		write (fd_tens_out, "print", strlen("print") + 1);
	}
}

void printUnits(){
	if(getExPid("units") != 0){
		write (fd_units_out, "print", strlen("print") + 1);
	}
}

void getTens(int led){
	if(getExPid("tens") != 0){
		char message[100];
		sprintf(message, "info %d", led);
		write (fd_tens_out, "message", strlen("message") + 1);
	}
}

void setTens(int led, char * color){
	if(getExPid("tens") != 0){
		char message[100];
		sprintf(message, "color %d %s", led, color);
		write (fd_tens_out, "message", strlen("message") + 1);
	}
}

void getUnits(int led){
	if(getExPid("units") != 0){
		char message[100];
		sprintf(message, "info %d", led);
		write (fd_units_out, "message", strlen("message") + 1);
	}
}

void setUnits(int led, char * color){
	if(getExPid("units") != 0){
		char message[100];
		sprintf(message, "color %d %s", led, color);
		write (fd_units_out, "message", strlen("message") + 1);
	}
}

int main(int argc, char *argv[]){
	system("killall -s SIGKILL units");
	system("killall -s SIGKILL tens");
	char comando[20];
	int secondi = -1;
	int led;
	char ledcolor[100];

	creazionePipe();

	do{	
		printf("\nComandi disponibili: \n-start <secondi>: Avvio countdown\n-elapsed: Secondi rimasti\n-stop: Ferma il conto alla rovescia\n-tens: Decine\n-units: Unità\n-tensled info <n>\n-quit\n");
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
				stop();
			}else if(strcmp(comando, "tens") == 0){
				printTens();
			}else if(strcmp(comando, "units") == 0){
				printUnits();
			}else if(strncmp(comando, "tensled info", 12) == 0){
				sscanf(comando, "tensled info %d", &led);
				getTens(led);
			}else if(strncmp(comando, "unitsled info", 13) == 0){
				sscanf(comando, "unitsled info %d", &led);
				getUnits(led);
			}else if(strncmp(comando, "tensled color", 13) == 0){
				sscanf(comando, "tensled color %d %s", &led, ledcolor);
				setTens(led, ledcolor);
			}else if(strncmp(comando, "unitsled color", 14) == 0){
				sscanf(comando, "unitsled color %d %s", &led, ledcolor);
				setUnits(led, ledcolor);
			}else if(strcmp(comando, "quit") != 0){
				printf("\ncomando errato\n");
			}
		}


		
	}while(!(strcmp(comando, "quit") == 0));

	closePipe();

	return 0;
}
