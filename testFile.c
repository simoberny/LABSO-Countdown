#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <signal.h>

const int segmenti[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0,},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};


int num =0;
		char word[2000];
		//
		char *string;

		




int main(int argc, char *argv[]){

	while(1){

		printf("AAAAAAAAAAAAAAAA\n");



	//decine

	int controlloD[10] = {1,1,1,1,1,1,1,1,1,1};

		//tens
	for (int i=1;i<8; i++){
		char comand[29];
		sprintf(comand, "cat ./tens_led_%d", i);

		FILE *ls = popen(comand, "r");
		char buf[256];
		while (fgets(buf, sizeof(buf), ls) != 0) {
	   	 	//printf("\n PID ( %s ) : %s", process, buf);
		}
		pclose(ls);



		//printf("segmento[%d]: %s",i, buf);
		if(strncmp("red",buf,3) == 0){
			//printf("red!\n");
			for(int j=0; j<10; j++){
				if(segmenti[j][i-1]==0){
					controlloD[j] = 0;
				}
			}
		}
		if(strncmp("off",buf,3) == 0){
			//printf("off!\n");
			for(int j=0; j<10; j++){
				if(segmenti[j][i-1]==1){
					controlloD[j] = 0;
				}
			}

		}
	}
	int de = -1;
		for(int j=0; j<10; j++){
				if(controlloD[j] == 1){
					de = j;
				}
					
		}
	if(de > -1){
		printf("%d ", de);
	} else {
		printf("E ");
	}






	int controlloU[10] = {1,1,1,1,1,1,1,1,1,1};

		//tens
	for (int i=1;i<8; i++){
		char comand[29];
		sprintf(comand, "cat ./units_led_%d", i);

		FILE *ls = popen(comand, "r");
		char buf[256];
		while (fgets(buf, sizeof(buf), ls) != 0) {
	   	 	//printf("\n PID ( %s ) : %s", process, buf);
		}
		pclose(ls);



		//printf("segmento[%d]: %s",i, buf);
		if(strncmp("red",buf,3) == 0){
			//printf("red!\n");
			for(int j=0; j<10; j++){
				if(segmenti[j][i-1]==0){
					controlloU[j] = 0;
				}
			}
		}
		if(strncmp("off",buf,3) == 0){
			//printf("off!\n");
			for(int j=0; j<10; j++){
				if(segmenti[j][i-1]==1){
					controlloU[j] = 0;
				}
			}

		}
	}
	int un = -1;
		for(int j=0; j<10; j++){
				if(controlloU[j] == 1){
					un = j;
				}
					
		}
	if(un > -1){
		printf("%d/n ", un);
	} else {
		printf("E/n");
	}


	


	/*if (controllo==0){
		print
	}*/

		/*//units
	for (int i=1;i<8; i++){
		char comand[29];
		sprintf(comand, "cat units_led_%d", i);

		FILE *ls = popen(comand, "r");
		char buf[256];
		while (fgets(buf, sizeof(buf), ls) != 0) {
	   	 	//printf("\n PID ( %s ) : %s", process, buf);
		}
		pclose(ls);
		printf("segmento[%d]: %s",i, buf);

	}*/


	//usleep(100000);
	//printf("\n");
	}
	exit(0);
}
