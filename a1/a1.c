#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

void listError(char* program_name) {
	printf("ERROR\n");
	printf("Invalid syntax\n");
	printf("Syntax: %s list [recursive] <filtering_options> path=<dir_path>\n", program_name);
	exit(-1);
}

void listOption(char* dirPath, bool recursive, bool has_perm_execute, int size_greater) {
	DIR* dir = opendir(dirPath);
	if (dir == NULL) {
		printf("ERROR\n");
		perror("");
		exit(-1);
	}
	char fullPath[512];
	struct dirent *entry = NULL;
	struct stat statbuf;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			snprintf(fullPath, 512, "%s/%s", dirPath, entry->d_name);
			if (lstat(fullPath, &statbuf) != 0) {
				continue;
			}
			if (S_ISREG(statbuf.st_mode) && (has_perm_execute == false || statbuf.st_mode & S_IXUSR) && (statbuf.st_size > size_greater)) {
				printf("%s\n", fullPath);
			}
			else if (S_ISDIR(statbuf.st_mode)) {
			 	if (size_greater == -1)
					printf("%s\n", fullPath);
			  	if (recursive == true)
					listOption(fullPath, recursive, has_perm_execute, size_greater);
			}
		}	
	}

	closedir(dir);
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		printf("ERROR\n");
		printf("Invalid number of arguments!\n");
		printf("Syntax: %s [OPTIONS] [PARAMETERS]\n", argv[0]);
		exit(-1);
	}
	char* cmd = argv[1];
	if (strcmp(cmd, "variant") == 0) {
		printf("87188\n");
		return 0;
	}
	if (strcmp(cmd, "list") == 0) {
		if (argc == 2) {
			listError(argv[0]);
		}
		char* path = NULL;
		bool recursive = false, has_perm_execute = false;
		int size_greater = -1;

		for (int i = 2; i < argc; i++) { // we are looking for the path
			if (strstr(argv[i], "path=") == argv[i]) {
				path = argv[i] + 5; // the path will start at the 5th position
			}
			else if (strcmp(argv[i], "recursive") == 0) {
				recursive = true;
			}
			else if (strcmp(argv[i], "has_perm_execute") == 0) {
				has_perm_execute = true;
			}
			else if (strstr(argv[i], "size_greater=") == argv[i]) {
				char* charValue = argv[i] + 13;
				bool isNumber = true;
				if (strlen(charValue) == 0) {
					isNumber = false;
				}
				else {
					for (int i = 0; i < strlen(charValue); i++) {
						if (charValue[i] < '0' || charValue[i] > '9') {
							isNumber = false;
							break;
						}	
					}
				}
				if (isNumber == false) {
					listError(argv[0]);
				}
				size_greater = atoi(charValue);
			}
			else {
				listError(argv[0]);
			}

		}
		if (path == NULL || strlen(path) == 0) {
			listError(argv[0]);
		}

		DIR* verifyValidPath = opendir(path);
		if (verifyValidPath == NULL) {
			printf("ERROR\n");
			perror("");
			exit(-1);
		}
		closedir(verifyValidPath);

		printf("SUCCESS\n");
		listOption(path, recursive, has_perm_execute, size_greater);
	}
	return 0;
}
