#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>
#include <sys/unistd.h>
#include <signal.h>

//Se la dafinizione TARGET esiste, creata se il make rileva un sistema Raspberry, include la libreria per raspberry
#if (defined TARGET)
	//Libreria utilizzata da Raspberry
	#include <wiringPi.h>
#endif  

#define READ 0
#define WRITE 1

int decine = -1;
int fd_tens_in;
int fd_tens_out;

pid_t pidFiglio[7];
int fds[7][2];

//Vettore con la funzione di tradurre un numero nel suo corrispettivo a 7 segmenti
const int segmenti[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0,},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};  //matrice che mappa ogni  segmento(riga) con ogni numero(colonna)
const int gpioTens[7]={3,12,30,14,13,0,2}; //mappa i pin della libreria wiring con i segmenti per le decine

//Funzione che chiude tutte le pipe, elimina tutti i processi figli relativi ai segmenti e termina il processo corrente
void closeAll(){
	for(int i = 0; i < 7; i++){
		kill(pidFiglio[i], SIGKILL);
	}
	close(fd_tens_in);
	close (fd_tens_out);
	unlink("tens_pipe_out");
	exit(0);
}

//Funzione che dato il nome di un processo restituisce un PID
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

//Handler che viene chiamato quando arriva il segnale inviato dal processo units che termina 
void killHandler(int sig){
	closeAll();
}

//Handler che viene chiamato quando il processo units arriva a 0 e invia il segnale alla decine affinchè si decrementino 
void countHandler (int sig) {
	//Se ci sono ancora decine vengono decrementate
	if(decine > 0){
		decine--;
	}

	//Aggiornamento dei singoli segmenti inviando le decine al processi figli
	for(int i = 0; i < 7; i++){		
		char m[100];	
		sprintf(m, "n %d non non", decine);
		write(fds[i][WRITE], m, strlen(m) + 1);		

		//Aggiornamento 7 segmenti sempre se la definizione Target esiste
		#if (defined TARGET)
			//gipo decremento!
			wiringPiSetup () ;
	  		pinMode (gpioTens[i], OUTPUT);
	  		digitalWrite (gpioTens[i], !segmenti[decine][i]);
  		#endif

  	}

  	//Se le decine arrivano a 0 il processo invia un segnale alle unità per avvertirle che il successivo è l'ultimo conteggio che devono eseguire
	if(decine == 0){
		kill(getExPid("units"), SIGUSR2);		
	}
}

//Funzione che crea i 7 figli per gestire i singoli segmenti
void creazioneFigli(char ** argv){

	int pid;

	for(int i = 0; i<7; i++){

		//Creazione pipe anonima per ogni figlio
		pipe(fds[i]);
		pid = fork();

		//Salvataggio dei pid dei figli
		pidFiglio[i] = pid;

		if (pid==0){
			//Processo figlio numero #i

			unlink("tens_pipe_out");
			close(fd_tens_in);
			close (fd_tens_out);

			FILE *fd;

			//Viene cambiato il nome dei figli
			int argv0size = strlen(argv[0]);
			strncpy(argv[0], "figliotens", argv0size);

			char messag[100];
			char comando[100];
			char tmpColor[50];

			//Variabile per salvare lo stato del segmento: "OFF se spento", "#COLORE se acceso"
			char stato[50];

			//Variabile che salva il colore del segmento indipendentemente dallo stato
			char colore[50];

			//Valori di default dei segmenti
			strcpy(stato, "off");
			strcpy(colore, "red");

			//Ciclo dei figli
			while (1){							
				int bytesRead;
				//Legge stringhe in arrivo sulla pipe
				while((bytesRead = read(fds[i][READ], messag, 100)) > 0){
					int valore=0;
					int led = 0;

					if(strncmp(messag, "n", 1) == 0){
						//Salvo il valore delle decine attuali, il comando richiesto se richiesto e un eventuale colore da settare
						sscanf(messag, "n %d %s %s", &valore, comando, tmpColor);

						//Controllo se questo segmento (Figlio #i) è acceso dato il numero attuale
						if(segmenti[valore][i] == 1){
							strcpy(stato, colore);
						}else{
							strcpy(stato, "off");
						}		

						//Salvo lo stato del segmento su file, nella cartella assets
						char directory[100];
						sprintf(directory, "../assets/tens_led_%d", i);
											
						fd=fopen(directory, "w");

						if(fd != NULL){
							fprintf(fd, "%s\n", stato);
  							fclose(fd);
						}
				
						if(strcmp(comando, "Info") == 0){
							//Richiesta stato segmento
							printf("Stato LED Decine%d: %s \n",i, stato);
						}else if(strcmp(comando, "Color") == 0){
							//Setto il colore di un dato segmento
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

//Legge stringhe in ingresso, carattere per carattere, dato il file descriptor della pipe
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

	//Apertura pipe di uscita per comunicare con il processo principale
	do {
		fd_tens_out = open ("tens_pipe_in", O_WRONLY);
	} while (fd_tens_out == -1);

	//Creazione pipe
	unlink("tens_pipe_out");
	mknod ("tens_pipe_out", S_IFIFO, 0);
	chmod ("tens_pipe_out", 0660); 
	fd_tens_in = open ("tens_pipe_out", O_RDONLY);
	
	while(readLine (fd_tens_in, str)) {
		sprintf(message, "%s", str);
		if(strncmp(message, "tens", 4) == 0){
			sscanf(message, "tens %d", &decine);

			//Vengono creati i 7 figli che gestiscono i segmenti
			creazioneFigli(argv);

			//Se le decine inserite sono già 0 viene inviato subito il segnale
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

		//Per ogni segmento viene generata la richiesta da effettuare 
		for(int i = 0; i < 7; i++){			
			if(led == i){
				sprintf(msgPip, "n %d %s", decine, richiesta);
				led = -1;
			}else{
				//Se non c'è nessuna richiesta vengono semplicemente inviate le decine attuali
				//Al posto della richiesta vengono inseriti due semplici "-" per skippare la richiesta
				sprintf(msgPip, "n %d - -", decine);
			}
			write(fds[i][WRITE], msgPip, strlen(msgPip) + 1);
			
			//Settaggio iniziale dei 7 segmenti
			#if (defined TARGET)
				wiringPiSetup();
				pinMode (gpioTens[i], OUTPUT);
				digitalWrite (gpioTens[i], !segmenti[decine][i]);
			#endif
			
		}
	}
}

