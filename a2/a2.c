#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include "a2_helper.h"

// 2.3 Parameters
pthread_cond_t cond1_3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1_3 = PTHREAD_MUTEX_INITIALIZER;
int thread7_2_has_started = 0;
pthread_cond_t cond2_3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2_3 = PTHREAD_MUTEX_INITIALIZER;
int thread7_1_has_ended = 0;

// 2.4 Parameters
sem_t sem_4;
int threads_running = 0;
int thread11_running = 0; // 0 if not runned, 1 if it's running and 2 if it ended
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1_4 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2_4 = PTHREAD_COND_INITIALIZER;

// 2.5 Parameters
sem_t* sem1_5, *sem2_5;

// 2.3 Function (and 2.5)
void* thread_function_3(void* arg) {
        int thread_no = *((int *)arg);
        pthread_mutex_lock(&mutex1_3);
        if(thread_no == 1) {
        	while (!thread7_2_has_started) 
			pthread_cond_wait(&cond1_3, &mutex1_3);
        }
    	else if (thread_no == 4) {
    		sem_wait(sem1_5);
    	}
        info(BEGIN, 7, thread_no);
        if (thread_no == 2) {
                thread7_2_has_started = 1;
                pthread_cond_broadcast(&cond1_3);
                pthread_mutex_unlock(&mutex1_3);
        }
        else
                pthread_mutex_unlock(&mutex1_3);
        pthread_mutex_lock(&mutex2_3);
        if(thread_no == 2) {
                while (!thread7_1_has_ended)
                        pthread_cond_wait(&cond2_3, &mutex2_3);
        }
        info(END, 7, thread_no);
        if (thread_no == 1) {
                thread7_1_has_ended = 1;
                pthread_cond_signal(&cond2_3);
        }
    	else if (thread_no == 4) {
    		sem_post(sem2_5);
    	}
    	pthread_mutex_unlock(&mutex2_3);
        return NULL;
}

// 2.4 Function
void* thread_function_4(void* arg) {
    	int thread_no = *((int *) arg);
    	sem_wait(&sem_4);
    	info(BEGIN, 8, thread_no);

    	pthread_mutex_lock(&mutex_4);
    	threads_running++;
    	if (thread_no == 11) {
    	        thread11_running = 1;
        	if (threads_running < 6) {
            		pthread_cond_wait(&cond1_4, &mutex_4);
        	}
    	}
    	else if (threads_running == 6) {
    		if (thread11_running == 1) {
    			pthread_cond_signal(&cond1_4);
    			if (thread11_running == 1) {
    				pthread_cond_wait(&cond2_4, &mutex_4);
    			}
    		}
    	}
    	else {
        	if (thread11_running < 2) {
            		pthread_cond_wait(&cond2_4, &mutex_4);
        	}
    	}
    	threads_running--;
    	info(END, 8, thread_no);
    	if (thread_no == 11) {
        	thread11_running = 2;
        	pthread_cond_broadcast(&cond2_4);
    	}
    	pthread_mutex_unlock(&mutex_4);
    	sem_post(&sem_4);
    	return NULL;
}

// 2.5 Function
void* thread_function_5(void* arg) {
    	int thread_no = *((int *) arg);
    	if (thread_no == 3) {
    		sem_wait(sem2_5);
    	}
        info(BEGIN, 5, thread_no);

    	info(END, 5, thread_no);
    	if (thread_no == 1) {
    		sem_post(sem1_5);
    	}
    	return NULL;
}

int main(int argc, char* argv[]){
    	init();
    	info(BEGIN, 1, 0);
	
	sem1_5 = sem_open("/a2c5_semaphore1", O_CREAT, 0644, 1);
	sem2_5 = sem_open("/a2c5_semaphore2", O_CREAT, 0644, 1);	
	
	
	
	// blocam semafoarele, thread-urile care se vor termina intr-un proces vor debloca semaforul, iar cele ce asteptau la el din celalalt proces vor putea trece	
	sem_wait(sem1_5);
	sem_wait(sem2_5);
	
	// cream ierarhie de procese
	int pid2 = fork();
    	if (pid2 == -1) {
        	perror("fork() error");
        	exit(-1);
    	}
    	else if (pid2 == 0) {
        	// P2
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
        	// P3
        	info(BEGIN, 3, 0);
        	int pid9 = fork();
        	if (pid9 == -1) {
                	perror("fork() error");
                	exit(-1);
        	}
        	else if (pid9 == 0) {
                	// P9
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
    	    	// P4
    	    	info(BEGIN, 4, 0);
    	    	int pid6 = fork();
    	    	if (pid6 == -1) {
    	            	perror("fork() error");
    	            	exit(-1);
        	}
        	else if (pid6 == 0) {
                	// P6
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
        	// P5
        	info(BEGIN, 5, 0);
       		int pid7 = fork();
        	if (pid7 == -1) {
                	perror("fork() error");
                	exit(-1);
        	}
        	else if (pid7 == 0) {
                	// P7
                	info(BEGIN, 7, 0);
                	int pid8 = fork();
                	if (pid8 == -1) {
                        	perror("fork() error");
                        	exit(-1);
                	}
                	else if (pid8 == 0) {
                        	// P8
                        	info(BEGIN, 8, 0);
                		pthread_t threads[36];
               			int params[36];
                		if (sem_init(&sem_4, 0, 6) != 0) {
                    			perror("sem_init() error");
               			}
                		for (int i = 0; i < 36; i++) {
                    			params[i] = i + 1;
                    			pthread_create(&threads[i], NULL, thread_function_4, &params[i]);
                		}
                		for (int i = 0; i < 36; i++) {
                    			pthread_join(threads[i], NULL);
                		}
                		sem_destroy(&sem_4);
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
        	pthread_t threads[5];
        	int params[5];
        	for (int i = 0; i < 5; i++) {
            		params[i] = i + 1;
            		pthread_create(&threads[i], NULL, thread_function_5, &params[i]);
        	}
        	for (int i = 0; i < 5; i++) {
            		pthread_join(threads[i], NULL);
        	}
        	wait(0);
        	info(END, 5, 0);
        	exit(0);
    	}
    	wait(0);
    	wait(0);
    	wait(0);
    	wait(0); // asteptam dupa cei 4 copii

	sem_close(sem1_5);
	sem_close(sem2_5);
	
	// stergem semafoarele din memorie
	sem_unlink("/a2c5_semaphore1");
	sem_unlink("/a2c5_semaphore2");
	
    	info(END, 1, 0);
    	return 0;
}
