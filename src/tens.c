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
	char message[100];
	unlink("tens_pipe");
	mknod ("tens_pipe", S_IFIFO, 0);
	chmod ("tens_pipe", 0660); 
	fd = open ("tens_pipe", O_RDONLY);
	while (readLine (fd, str)) 
		sprintf(message, "touch tens.txt && echo '%s' >> tens.txt", str);
		system(message);
	close (fd);
	unlink("tens_pipe");
}

