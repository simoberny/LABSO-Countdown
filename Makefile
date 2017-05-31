ODIR = build
SDIR = src
secondi = 0
sistema := "Linux"
CC := gcc

ifeq ($(findstring arm, $(shell uname -m)), arm)
   sistema := "Rasp"
   CC := gcc-6 -DTARGET=RASP -lwiringPi
endif

#Stampa informazioni gruppo e progetto
help: 
	@echo "----------------------------\n\nAndrea Filippi 181598 \nMatteo Dal Ponte 181390 \nSimone Bernabè 181046\n\n---------------------------\n\nProgetto 2 CountDown LED\n Programma C che utilizzando la sicnronizazzione tra processi crea un conto alla rovescia con i parametri letti in ingresso"

#Elimina file oggetto e assets nella cartella build
clean:
	@rm -rf build && if [ -d "assets" ]; then rm -rf assets; fi && echo "\033[92m - Clean eseguito con successo! \033[0m"

#Deve creare la cartella build con dentro il file eseguibile
build: $(SDIR)/progetto.c $(SDIR)/units.c $(SDIR)/tens.c
	@mkdir build && \
	$(CC) $(SDIR)/progetto.c -o countdown && \
	$(CC) $(SDIR)/tens.c -o tens && \
	$(CC) $(SDIR)/units.c -o units && \
	mv countdown  build/ && \
	mv tens build/ && \
	mv units build/ && \
	echo "\033[92m - Compilazione terminata con successo! \033[0m"

#OPZIONALE - Chiama build e crea una cartella assets con i file di supporto
assets:  
	@mkdir assets && make -s build && \
	for units in 0 1 2 3 4 5 6; do \
		touch assets/units_led_$$units ; \
	done && \
	for tens in 0 1 2 3 4 5 6; do \
		touch assets/tens_led_$$tens ; \
	done && \
	echo "\033[92m - Assets creati! \033[0m"
		
	
#OPZIONALE - Esegue il progetto in modalità TEST richiamando prima "assets" e chiedendo i secondi da bash
test: 
	@make -s assets && \
	read -p "Inserire secondi:" secondi; cd build && ./countdown $$secondi
