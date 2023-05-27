#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>

#define fifo1 "RESP_PIPE_87188"
#define fifo2 "REQ_PIPE_87188"


void writeString(int fd, char* str) {
	int size = strlen(str);
	write(fd, &size, 1);
	write(fd, str, size);
}

void writeInt(int fd, int num) {
	write(fd, &num, sizeof(int));
}

int main(int argc, char* argv[]) {
	unlink(fifo1);
	if (mkfifo(fifo1, 0600) != 0) {
		printf("ERROR\n"); 
		printf("cannot create the response pipe\n"); 
		return -1;
	}
	int fd1 = -1, fd2 = -1;
	fd2 = open(fifo2, O_RDONLY); 
	if (fd2 == -1) { 
		printf("ERROR\n"); 
		printf("cannot open the request pipe\n");
		unlink(fifo1);
		return -1;
	}
	fd1 = open(fifo1, O_WRONLY);
	if (fd1 == -1) { 
		printf("ERROR\n");
		printf("cannot open the response pipe\n"); 
		unlink(fifo1);
		return -1;
	}
	writeString(fd1, "HELLO");
	printf("SUCCESS\n");
	ssize_t size;
	volatile char* sharedMem = NULL;
	int nrOcteti = 0;
	off_t fileSize;
	char* data = NULL; 
	int shmFd;
	
	while(1) { 
		//citesc lungimea mesajului 
		int strLength = 0;
		size = read(fd2, &strLength, 1); 
		if (size < 0) { 
			perror("error reading from pipe"); 
			return -1;
		}
		char msg[strLength + 1];
		size = read(fd2, msg, strLength); 
		if (size < 0) {
			perror("error reading from pipe"); 
			return -1;
		}
		msg[strLength] = '\0';
		
		if (strcmp(msg, "PING") == 0) { 
			writeString(fd1, "PING");
			writeString(fd1, "PONG");
			writeInt(fd1, 87188);
		}
		else if (strcmp(msg, "CREATE_SHM") == 0) {  
			read(fd2, &nrOcteti, sizeof(int));
		
			shmFd = shm_open("/l2DqaHC", O_CREAT|O_RDWR, 0664); 
			if (shmFd < 0) { 
				//scriem inapoi mesajul "CREATE_SHM" "ERROR" 
				writeString(fd1, "CREATE_SHM");
				writeString(fd1, "ERROR");
				continue;
			}
			ftruncate(shmFd, nrOcteti);
			sharedMem = (volatile char*)mmap(0, nrOcteti, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
			if (sharedMem == (void*)-1) { 
				//scriem inapoi mesajul "CREATE_SHM" "ERROR"
				writeString(fd1, "CREATE_SHM");
				writeString(fd1, "ERROR");
				shm_unlink("/l2DqaHC");
				continue;
			}
			//scriem mesajul de succes
			writeString(fd1, "CREATE_SHM");
			writeString(fd1, "SUCCESS");
		}
		else if (strcmp(msg, "EXIT") == 0) { 
			//inchidem pipe-ul request, se inchide si se sterge pipe-ul response si programul isi termina executia 
			close(fd1); 
			close(fd2); 
			unlink(fifo1); 
			sharedMem = NULL;
			shm_unlink("/l2DqaHC");
			break;
			
		}
		else if (strcmp(msg, "MAP_FILE") == 0) { 
			int nr = 0;
			//citim numarul de litere din numele fisierului 
			read(fd2, &nr, 1); 
			char fileName[nr + 1];
			read(fd2, fileName, nr); 
			fileName[nr] = '\0';
			int fd = -1;
			fd = open(fileName, O_RDONLY); //doar in citire 
			if (fd == -1) { 
				//map file error 
				writeString(fd1, "MAP_FILE");
				writeString(fd1, "ERROR");
				continue;
			}
			//aflu dimensiunea fisierului 
			fileSize = lseek(fd, 0, SEEK_END); 
			lseek(fd, 0, SEEK_SET);//ducem cursorul inapoi la inceput 
			
			data = (char*)mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0); //mapam fisierul 
			if (data == (void*)-1) { 
				//map file error 
				writeString(fd1, "MAP_FILE");
				writeString(fd1, "ERROR");
				continue;
			}
			//scriem inapoi mesajul de succes
			writeString(fd1, "MAP_FILE");
			writeString(fd1, "SUCCESS");
			
		}
		else if (strcmp(msg, "WRITE_TO_SHM") == 0) { 
			unsigned int offset = 0;
			unsigned int value = 0;
			ssize_t size1, size2;
			size1 = read(fd2, &offset, sizeof(unsigned int));
			size2 = read(fd2, &value, sizeof(unsigned int));
			if (size1 < 0 || size2 < 0) { 
				perror("error reading from pipe"); 
				return -1;
			}
			if (offset < 0 || offset > nrOcteti) {
				writeString(fd1, "WRITE_TO_SHM");
				writeString(fd1, "ERROR"); 
				continue;
			}
			else if (offset + sizeof(unsigned int) > nrOcteti) {
				writeString(fd1, "WRITE_TO_SHM");
				writeString(fd1, "ERROR"); 
				continue;
			}
			//scriem la offset-ul respectiv valoarea value 
			*((unsigned int*)(sharedMem + offset)) = value;
			
			//transmitem mesajul de succes
			writeString(fd1, "WRITE_TO_SHM");
			writeString(fd1, "SUCCESS"); 				
		}
		else if (strcmp(msg, "READ_FROM_FILE_OFFSET") == 0) {
			unsigned int offset, noBytes;
			ssize_t size1, size2; 
			size1 = read(fd2, &offset, sizeof(unsigned int)); 
			size2 = read(fd2, &noBytes, sizeof(unsigned int)); 
			if (size1 < 0 || size2 < 0) { 
				perror("error reading from pipe"); 
				return -1;
			} 
		
			if (sharedMem == NULL || data == NULL || noBytes + offset >= fileSize) { //daca nu exista o zona de memorie partajata sau un fisier mapat 
				//in memorie, afisam un mesaj de eraore
        			writeString(fd1, "READ_FROM_FILE_OFFSET");
        			writeString(fd1, "ERROR");
        			continue;	
			}
			char* dataOffset = data + offset;
			for (int i = 0; i < noBytes; i++) { 
				sharedMem[i] = dataOffset[i];
			}
			//strncpy(sharedMem, dataOffset, noBytes); 
			writeString(fd1, "READ_FROM_FILE_OFFSET");
			writeString(fd1, "SUCCESS");
		}
		else if (strcmp(msg, "READ_FROM_FILE_SECTION") == 0) { 
			unsigned int sectionNo, offset, noBytes; 
			ssize_t size1, size2, size3; 
			size1 = read(fd2, &sectionNo, sizeof(unsigned int)); 
			size2 = read(fd2, &offset, sizeof(unsigned int)); 
			size3 = read(fd2, &noBytes, sizeof(unsigned int)); 
			if (size1 < 0 || size2 < 0 || size3 < 0) {
				perror("error reading from pipe"); 
				return -1;
			}
			if (sharedMem == NULL || data == NULL) {
				writeString(fd1, "READ_FROM_FILE_SECTION");
        			writeString(fd1, "ERROR");
        			continue;
			}
			char noSections = *(data + 8);
			if (sectionNo > noSections) {
				writeString(fd1, "READ_FROM_FILE_SECTION");
        			writeString(fd1, "ERROR");
				continue;
			}
			int sectionOffset = *((int*)(data + 9 + 15 * (sectionNo - 1) + 7));
			int sectionSize = *((int*)(data + 9 + 15 * (sectionNo - 1) + 11));
			if (sectionOffset + offset + noBytes > sectionOffset + sectionSize) {
				writeString(fd1, "READ_FROM_FILE_SECTION");
        			writeString(fd1, "ERROR");
        			continue;
			}
			char* dataOffset = data + sectionOffset + offset;
			for (int i = 0; i < noBytes; i++) {
				sharedMem[i] = dataOffset[i];
			}
			writeString(fd1, "READ_FROM_FILE_SECTION");
			writeString(fd1, "SUCCESS");
		}
		else if (strcmp(msg, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0) { 
			unsigned int logicalOffset, noBytes; 
			ssize_t size1, size2; 
			size1 = read(fd2, &logicalOffset, sizeof(unsigned int)); 
			size2 = read(fd2, &noBytes, sizeof(unsigned int)); 
			if (size1 < 0 || size2 < 0) {
				perror("error reading from pipe"); 
				return -1;
			}
			if (sharedMem == NULL || data == NULL) {
				writeString(fd1, "READ_FROM_LOGICAL_SPACE_OFFSET");
        			writeString(fd1, "ERROR");
        			continue;
			}
			char noSections = *(data + 8);
			int logicalSectionsOffset = 0, done = 0;
			for (int section = 1; section <= noSections; section++) {
				int sectionOffset = *((int*)(data + 9 + 15 * (section - 1) + 7));
				int sectionSize = *((int*)(data + 9 + 15 * (section - 1) + 11));
				int logicalSectionSize = (sectionSize - sectionSize % 1024) + (sectionSize % 1024 == 0 ? 0 : 1024);
				if (logicalOffset <= logicalSectionsOffset + logicalSectionSize) {
					if (sectionOffset + (logicalOffset - logicalSectionsOffset) > fileSize) {
						break;
					}
					char* dataOffset = data + sectionOffset + (logicalOffset - logicalSectionsOffset);
					for (int i = 0; i < noBytes; i++) {// sigur stiu ce fac
						sharedMem[i] = dataOffset[i];
					}
					done = 1;
					break;
				}
				logicalSectionsOffset += logicalSectionSize;
			}
			if (!done) {
				writeString(fd1, "READ_FROM_LOGICAL_SPACE_OFFSET");
        			writeString(fd1, "ERROR");
        			continue;
			}
			writeString(fd1, "READ_FROM_LOGICAL_SPACE_OFFSET");
			writeString(fd1, "SUCCESS");
		}
				
	}		
	return 0;
}

