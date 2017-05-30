ODIR = build
SDIR = src
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
	@rm -rf build && if [ -d "assets" ]; then rm -rf assets; fi && echo "Build e Assets eliminate! "

#Deve creare la cartella build con dentro il file eseguibile
build: $(SDIR)/progetto.c $(SDIR)/units.c $(SDIR)/tens.c
	@mkdir build && \
	$(CC) $(SDIR)/progetto.c -o countdown && \
	$(CC) $(SDIR)/tens.c -o tens && \
	$(CC) $(SDIR)/units.c -o units && \
	mv countdown  build/ && \
	mv tens build/ && \
	mv units build/

#OPZIONALE - Chiama build e crea una cartella assets con i file di supporto
assets:  
	@mkdir assets && make build && \
	for units in 1 2 3 4 5 6 7; do \
		touch assets/units_led_$$units ; \
	done && \
	for tens in 1 2 3 4 5 6 7; do \
		touch assets/tens_led_$$tens ; \
	done
		
	
#OPZIONALE - Esegue il progetto in modalità TEST richiamando prima "assets"
test: 
	@make assets && echo "TESTING" && cd build && ./countdown 35
