#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "a2_helper.h"

pthread_cond_t cond1_3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1_3 = PTHREAD_MUTEX_INITIALIZER;
int thread2_has_started = 0;
pthread_cond_t cond2_3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2_3 = PTHREAD_MUTEX_INITIALIZER;
int thread1_has_ended = 0;

void* thread_function_3(void* arg) {
	int thread_no = *((int*)arg);
	pthread_mutex_lock(&mutex1_3);
	if(thread_no == 1) {
		while (!thread2_has_started) 
			pthread_cond_wait(&cond1_3, &mutex1_3);
	}
	info(BEGIN, 7, thread_no);
	if (thread_no == 2) {
		thread2_has_started = 1;
		pthread_cond_broadcast(&cond1_3);
		pthread_mutex_unlock(&mutex1_3);
	}
	else
		pthread_mutex_unlock(&mutex1_3);
	pthread_mutex_lock(&mutex2_3);
	if(thread_no == 2) {
		while (!thread1_has_ended) 
			pthread_cond_wait(&cond2_3, &mutex2_3);
	}
	info(END, 7, thread_no);
	if (thread_no == 1) {
		thread1_has_ended = 1;
		pthread_cond_broadcast(&cond2_3);
		pthread_mutex_unlock(&mutex2_3);
	}
	else
		pthread_mutex_unlock(&mutex2_3);
	return NULL;
}

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
    		pthread_t threads[4];
    		int params[4];
    		for (int i = 0; i < 4; i++) {
    			params[i] = i + 1;
    			pthread_create(&threads[i], NULL, thread_function_3, &params[i]);
    		}
    		for (int i = 0; i < 4; i++) {
    			pthread_join(threads[i], NULL);
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
