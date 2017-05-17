ODIR = build
SDIR = src

#Stampa informazioni gruppo e progetto
help: 
	@echo "----------------------------\n\nAndrea Filippi 181598 \nMatteo Dal Ponte 181390 \nSimone Bernabè 181046\n\n---------------------------\n\nProgetto 2 CountDown LED\n Programma C che utilizzando la sicnronizazzione tra processi crea un conto alla rovescia con i parametri letti in ingresso"

#Elimina file oggetto e assets nella cartella build
clean:
	@rm -rf build && if [ -d "assets" ]; then rm -rf assets; fi && echo "Build e Assets eliminate! "

#Deve creare la cartella build con dentro il file eseguibile
build: $(SDIR)/progetto.c $(SDIR)/progetto.h $(SDIR)/units.c $(SDIR)/tens.c
	@mkdir build && gcc $(SDIR)/progetto.c $(SDIR)/progetto.h -o countdown && gcc $(SDIR)/tens.c -o tens && mv countdown  build/ && mv tens build/

#OPZIONALE - Chiama build e crea una cartella assets con i file di supporto
assets:  
	make build && mkdir assets 

#OPZIONALE - Esegue il progetto in modalità TEST richiamando prima "assets"
test: 
	make assets && echo "TESTING"
