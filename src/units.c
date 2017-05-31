#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/unistd.h>
#include <time.h>
#include <signal.h>

//La definizione TARGET viene abilitata dal makefile in base alla piattaforma rilevata
#if (defined TARGET)
	//Libreria utilizzata da Raspberry
	#include <wiringPi.h>
#endif  

#define READ 0
#define WRITE 1

int unita = -1;
int unita_res = 0;
int decine = 1;
int fd_units_in;
int fd_units_out;

pid_t pidFiglio[7];

//Vettore di file descriptor per gestire le pipe anonime dei segmenti
int fds[7][2];

time_t end, start;

//Matrice che dato il numero di decine e il segmento restituisce lo stato (on/off)
const int segmenti[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0,},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};  //matrice che mappa ogni  segmento(riga) con ogni numero(colonna)
//mappa i pin della libreria wiring con i segmenti per le decine
const int gpioUnits[7]={16,1,21,23,25,15,22};

//Funzione che chiude tutte le pipe, elimina tutti i processi figli relativi ai segmenti e termina il processo corrente
void closeAll(){
	int i;
	for(i = 0; i < 7; i++){
		char closing[100];
		sprintf(closing, "n 0 non non");
		write(fds[i][WRITE], closing, strlen(closing) + 1);
		usleep(100000);
		kill(pidFiglio[i], SIGKILL);
	}
	close(fd_units_in);
	close (fd_units_out);
	unlink("units_pipe_out");
	exit(0);
}

//Funzione che dato il nome di un processo restituisce il suo PID, 0 altrimenti
int getExPid(char* process){
	char comand[29];
	sprintf(comand, "pidof -s %s", process);

	FILE *ls = popen(comand, "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), ls) != 0) {
	}
	pclose(ls);
	return atoi(buf);
}

//Handler che gestisce il segnale che manda le decine quando sono finite
void countHandler (int sig) { 
	decine = 0; // Permette alle unità di eseguire un ultimo ciclo;
}

//Funzione che crea i 7 figli per gestire i singoli segmenti
void creazioneFigli(char ** argv){
	int pid;
	int i;
	for(i = 0; i<7; i++){
		//Creazione pipe anonima per ogni figlio
		pipe(fds[i]);
		pid = fork();

		//Salvataggio dei pid dei figli
		pidFiglio[i] = pid;

		if (pid==0){
			//Processo figlio #i

			unlink("units_pipe_out");
			close(fd_units_in);
			close (fd_units_out);

			FILE *fd;

			//Viene cambiato il nome dei figli
			int argv0size = strlen(argv[0]);
			strncpy(argv[0], "figliounits", argv0size);

			char messag[100];
			char comando[100];
			char tmpColor[50];

			//Variabile per salvare lo stato del segmento: "OFF se spento", "COLORE se acceso"
			char stato[50];

			//Variabile che salva il colore del segmento indipendentemente dallo stato
			char colore[50];

			//Valori di default dei segmenti
			strcpy(stato, "off"); //Stato di ogni segmento inizializzato come spento
			strcpy(colore, "red"); //Colore di ogni segmento inizializzato a RED

			//Ciclo dei figli
			while (1){
				int bytesRead;
				//Legge stringhe in arrivo sulla pipe
				while((bytesRead = read(fds[i][READ], messag, 100)) > 0){
					int valore=0;
					int led = 0;

					if(strncmp(messag, "n", 1) == 0){

						//Salvo il valore delle unità attuali, il comando richiesto (se presente) e un eventuale colore da settare
						sscanf(messag, "n %d %s %s", &valore, comando, tmpColor);

						//Controllo se questo segmento (Figlio #i) è acceso dato il numero attuale
						if(segmenti[valore][i] == 1){
							strcpy(stato, colore);
						}else{
							strcpy(stato, "off");
						}

						// Salvo lo stato del segmento su file
						char directory[100];
						sprintf(directory, "../assets/units_led_%d", i+1);
						fd=fopen(directory, "w");
						if(fd != NULL){
							fprintf(fd, "%s\n", stato);
  							fclose(fd);
						}

						if(strcmp(comando, "Info") == 0){
							//Richiesta stato segmento
							printf("Stato LED Unità %d: %s \n",i+1, stato);
						}else if(strcmp(comando, "Color") == 0){
							//Setto il colore di un dato segmento
							strcpy(colore, tmpColor);
							printf("Colore segmento %d settato a: %s\n", i+1, colore);
						}
					}
				}

			}

			exit(0);
		}	
	}

	//NOTA: Handler dichiarati solo per il padre
	void countHandler (int);
	signal (SIGUSR2, countHandler);
}

//Legge stringhe in ingresso, carattere per carattere, dato il file descriptor della pipe
int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

int main(int argc, char ** argv){	
	char unita_str[10];

	char str[100];
	char message[100];
	char readcolor[50];

	char richiesta[100];
	char msgPip[100];

	clock_t start, end;
	int prevdiff = 0;
	int led = -1;

	//Apertura pipe di uscita per comunicare con il processo principale
	do {
		fd_units_out = open ("units_pipe_in", O_WRONLY);
	} while (fd_units_out == -1);

	//Creazione pipe
	unlink("units_pipe_out");
	mknod ("units_pipe_out", S_IFIFO, 0);
	chmod ("units_pipe_out", 0660); 
	fd_units_in = open ("units_pipe_out", O_NONBLOCK);

	while(1){	
		if (readLine (fd_units_in, str)) {
			sprintf(message, "%s", str);
	
			if(strncmp(message, "units", 5) == 0){
				sscanf(message, "units %d", &unita);

				//Vengono creati i 7 figli che gestiscono i segmenti
				creazioneFigli(argv);

				//Restituisce i secondi dalla prima build di Unix
				start = time(NULL);
			}else if(strcmp(message, "elapsed") == 0){
				sprintf(unita_str, "%d", unita);
				write (fd_units_out, unita_str, strlen(unita_str) + 1);
			}else if(strcmp(message, "stop") == 0){
				closeAll();
			}else if(strcmp(message, "print") == 0){
				char a = 'a';
				int i;
				for(i = 0; i < 7; i++){
					a = 'a' + i;
					printf("%c: %d\n",a,segmenti[unita][i]);
				}
			}else if(strncmp(message, "info", 4) == 0){
				sscanf(message, "info %d", &led);
				sprintf(richiesta, "Info");
			}else if(strncmp(message, "color", 5) == 0){
				sscanf(message, "color %d %s", &led, readcolor);
				sprintf(richiesta, "Color %s", readcolor);
			}
		}

		if(unita >=0){ //Se ci sono unità le decremento	

			//Per ogni segmento viene generata e inviata la richiesta da effettuare sulla rispettiva pipe anonima	
			int i;	
			for(i = 0; i < 7; i++){	

				//Settaggio dei 7 segmenti
				#if (defined TARGET)
					wiringPiSetup ();
	  				pinMode (gpioUnits[i], OUTPUT);			
					digitalWrite (gpioUnits[i], !segmenti[unita][i]);
				#endif

				if(led == i){
					sprintf(msgPip, "n %d %s", unita, richiesta);
					led = -1;
				}else{
					//Se non c'è nessuna richiesta (info #segmento o color #segmento colore) vengono semplicemente inviate le unità attuali
					sprintf(msgPip, "n %d non non", unita);
				}
				write(fds[i][WRITE], msgPip, strlen(msgPip) + 1);
			}

			//Ritardo per evitare di eseguire il programma a ciclo macchina(CPU 100%) inutilmente
			usleep(100000);

			//Controllo se è passato più di un secondo
			end=time(NULL);
			
			if(end-start>=1){	
				if(decine > 0){			
					if(unita == 0){ // Se le decine sono > 0 e sono arrivato a 0 con le unità invio segnale alle decine per decrementarsi
						if(getExPid("tens") != 0){
							kill(getExPid("tens"), SIGUSR1);
						}else{
							closeAll();
						}
					}
				}
				
				if(unita==0){							
					unita = 9;
				}else{				
					unita -= (end-start); 
				}
				start = end;
			}
				
		}
 
 		//Timer completato
		if(decine == 0 && unita == 0){
			printf("Timer completato!\n");

			//Viene inviato un segnale a tens per terminare la sua esecuzione
			if(getExPid("tens") != 0){
				kill(getExPid("tens"), SIGKILL);
			}

			#if (defined TARGET)
				int i;
				for(i = 0; i < 7; i++){	
					wiringPiSetup ();
					pinMode (gpioUnits[i], OUTPUT);			
					digitalWrite (gpioUnits[i], !segmenti[unita][i]);
				}
			#endif

			closeAll();
		}
		
	}
}

