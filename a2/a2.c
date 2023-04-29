#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

int main(){
    init();
    info(BEGIN, 1, 0);
    int pid2 = fork();
    if (pid2 == -1) {
    	perror("fork() error");
    	exit(-1);
    }
    else if (pid2 == 0) {
    	//P2
    	info(BEGIN, 2, 0);
    	info(END, 2, 0);
    	exit(0);
    }
    int pid3 = fork();
    if (pid3 == -1) {
    	perror("fork() error");
    	exit(-1);
    }
    else if (pid3 == 0) {
    	//P3
    	info(BEGIN, 3, 0);
    	int pid9 = fork();
    	if (pid9 == -1) {
    		perror("fork() error");
    		exit(-1);
    	}
    	else if (pid9 == 0) {
    		//P9
    		info(BEGIN, 9, 0);
    		info(END, 9, 0);
    		exit(0);
    	}
    	wait(0);
    	info(END, 3, 0);
    	exit(0);
    }
    
    int pid4 = fork();
    if (pid4 == -1) {
    	perror("fork() error");
    	exit(-1);
    }
    else if (pid4 == 0) {
    	//P4
    	info(BEGIN, 4, 0);
    	int pid6 = fork();
    	if (pid6 == -1) {
    		perror("fork() error");
    		exit(-1);
    	}
    	else if (pid6 == 0) {
    		//P6
    		info(BEGIN, 6, 0);
    		info(END, 6, 0);
    		exit(0);
    	}
    	wait(0);
    	info(END, 4, 0);
    	exit(0);
    }
    
    int pid5 = fork();
    if (pid5 == -1) {
    	perror("fork() error");
    	exit(-1);
    }
    else if (pid5 == 0) {
    	//P5
    	info(BEGIN, 5, 0);
    	int pid7 = fork();
    	if (pid7 == -1) {
    		perror("fork() error");
    		exit(-1);
    	}
    	else if (pid7 == 0) {
    		//P7
    		info(BEGIN, 7, 0);
    		int pid8 = fork();
    		if (pid8 == -1) {
    			perror("fork() error");
    			exit(-1);
    		}
    		else if (pid8 == 0) {
    			//P8
    			info(BEGIN, 8, 0);
    			info(END, 8, 0);
    			exit(0);
    		}
    		wait(0);
    		info(END, 7, 0);
    		exit(0);
    	}
    	wait(0);
    	info(END, 5, 0);
    	exit(0);
    }
    wait(0);
    wait(0);
    wait(0);
    wait(0); //astept dupa cei 4 copii
    info(END, 1, 0);
    return 0;
}
