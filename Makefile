ODIR = build
SDIR = src
secondi = 0
sistema := "Linux"
CC := gcc


#Controllo l'architettura per la compilazione
ifeq ($(findstring arm, $(shell uname -m)), arm)
   sistema := "Rasp"
   #DTARGET abilita la parte del codice riferita alla libreria wiringPi
   #NOTA: Versioni datate di gcc(ad esempio la 4.9 preinstallata su Raspberry) hanno problemi con le PIPE
   CC := gcc -DTARGET=RASP -lwiringPi
endif

#Stampa informazioni gruppo e progetto
help: 
	@echo "----------------------------\n\nAndrea Filippi 181598 \nMatteo Dal Ponte 181390 \nSimone Bernabè 181046\n\n---------------------------\n\nProgetto 2 CountDown LED\n Programma C che utilizzando la sicnronizazzione tra processi crea un conto alla rovescia con i parametri letti in ingresso\n\n" && \
	echo "Regole utilizzabili: \n -Clean\n -Build\n -Assets\n -Test\n -Run\n"
#Elimina file oggetto e assets nella cartella build
clean:
	@rm -rf build && if [ -d "assets" ]; then rm -rf assets; fi && echo "\033[92m - Clean eseguito con successo! \033[0m"

#Deve creare la cartella build con dentro il file eseguibile
build: $(SDIR)/progetto.c $(SDIR)/units.c $(SDIR)/tens.c
	@make -s clean && \
	mkdir build && \
	$(CC) $(SDIR)/progetto.c -o countdown && \
	$(CC) $(SDIR)/tens.c -o tens && \
	$(CC) $(SDIR)/units.c -o units && \
	mv countdown  build/ && \
	mv tens build/ && \
	mv units build/ && \
	echo "\033[92m - Compilazione terminata con successo! \033[0m"

#OPZIONALE - Chiama build e crea una cartella assets con i file di supporto
assets:  
	@make -s build && mkdir assets && \
	for units in 1 2 3 4 5 6 7 ; do \
		touch assets/units_led_$$units ; \
	done && \
	for tens in 1 2 3 4 5 6 7 ; do \
		touch assets/tens_led_$$tens ; \
	done && \
	echo "\033[92m - Assets creati! \033[0m"
		
	
#OPZIONALE - Esegue il progetto in modalità TEST richiamando prima "assets" e chiedendo i secondi da bash
test: 
	@make -s assets && \
	read -p "Inserire secondi:" secondi; cd build && ./countdown $$secondi

run:
	@cd build && ./countdown
