#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> /* For S_IFIFO */
#include <fcntl.h>

int readLine(int fd, char *str){
	int n; 
	do{
		n = read(fd, str, 1);
	}while(n>0 && *str++ != '\0');

	return n>0;
}

int main(){
	int fd;
	char str[100];
	unlink("units_pipe");
	mknod ("units_pipe", S_IFIFO, 0);
	chmod ("units_pipe", 0660); 
	fd = open ("units_pipe", O_RDONLY);
	while (readLine (fd, str)) 
		system("touch secondi.txt");
	close (fd);
	unlink("units_pipe");
}

