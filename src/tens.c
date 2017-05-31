#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/unistd.h>
#include <signal.h>

//La definizione TARGET viene abilitata dal makefile in base alla piattaforma rilevata
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

//Vettore di file descriptor per gestire le pipe anonime dei segmenti
int fds[7][2];

//Matrice che dato il numero di decine e il segmento restituisce lo stato (on/off)
const int segmenti[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0,},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};  //matrice che mappa ogni  segmento(riga) con ogni numero(colonna)
//mappa i pin della libreria wiring con i segmenti per le decine
const int gpioTens[7]={3,12,30,14,13,0,2}; 

//Funzione che chiude tutte le pipe, elimina tutti i processi figli relativi ai segmenti e termina il processo corrente
void closeAll(){
	for(int i = 0; i < 7; i++){
		char closing[100];
		sprintf(closing, "n 0 non non");
		write(fds[i][WRITE], closing, strlen(closing) + 1);
		usleep(100000);
		kill(pidFiglio[i], SIGKILL);
	}
	close(fd_tens_in);
	close (fd_tens_out);
	unlink("tens_pipe_out");
	exit(0);
}

//Funzione che dato il nome di un processo restituisce il suo PID, 0 altrimenti
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

//Handler che viene eseguito quando arriva il segnale inviato dal processo units quando termina
void killHandler(int sig){
	closeAll();
}

//Handler che decrementa le decine quando le unità arrivano a 0
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

		//Aggiornamento 7 segmenti
		#if (defined TARGET)
			//gipo decremento!
			wiringPiSetup () ;
	  		pinMode (gpioTens[i], OUTPUT);
	  		digitalWrite (gpioTens[i], !segmenti[decine][i]);
  		#endif

  	}

  	//Se le decine arrivano a 0 il processo invia un segnale alle unità per avvertirle di eseguire l'ultimo conteggio
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
			//Processo figlio #i

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

			//Variabile per salvare lo stato del segmento: "OFF se spento", "COLORE se acceso"
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
						//Salvo il valore delle decine attuali, il comando richiesto (se presente) e un eventuale colore da settare
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

	//NOTA: Handler dichiarati solo per il padre

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

			//Se le decine inserite sono già 0 viene inviato subito il segnale alle unità
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

		//Per ogni segmento viene generata e inviata la richiesta da effettuare sulla rispettiva pipe anonima
		for(int i = 0; i < 7; i++){			
			if(led == i){
				sprintf(msgPip, "n %d %s", decine, richiesta);
				led = -1;
			}else{
				//Se non c'è nessuna richiesta (info #segmento o color #segmento colore) vengono semplicemente inviate le decine attuali
				sprintf(msgPip, "n %d non non", decine);
			}
			write(fds[i][WRITE], msgPip, strlen(msgPip) + 1);
			
			//Settaggio dei 7 segmenti
			#if (defined TARGET)
				wiringPiSetup();
				pinMode (gpioTens[i], OUTPUT);
				digitalWrite (gpioTens[i], !segmenti[decine][i]);
			#endif
			
		}
	}
}

