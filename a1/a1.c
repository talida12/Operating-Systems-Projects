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

void listFunction(int argc, char* argv[]) {
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

void fileSizeError() {
	printf("ERROR\ninvalid file size\n");
	exit(-1);
}

typedef struct {
	char sect_name[7];
	char sect_type;
	int sect_offset;
	int sect_size;
} section_header;

section_header* parseSectionFile(int fd, unsigned short* header_size, unsigned short* version, unsigned char* number_of_sections) {
		char magic[5];
		if (read(fd, magic, 4*sizeof(char)) < 4) 
			fileSizeError();
			
		magic[4] = '\0';
		if (strcmp("bqfy", magic) != 0) {
			printf("ERROR\nwrong magic\n");
			exit(-1);
		}
		
		if (read(fd, header_size, sizeof(short)) < 2) 
			fileSizeError();
		
		if (read(fd, version, sizeof(short)) < 2) 
			fileSizeError();
		
		if (*version < 107 || *version > 200) {
			printf("ERROR\nwrong version\n");
			exit(-1);
		}
		
		if (read(fd, number_of_sections, sizeof(char)) < 1 ) 
			fileSizeError();
		
		if (*number_of_sections < 8 || *number_of_sections > 15) {
			printf("ERROR\nwrong sect_nr\n");
			exit(-1);
		}
		
		section_header* sh = (section_header*)malloc(*number_of_sections * sizeof(section_header));
		if (sh == NULL) {
			perror("ERROR\n");
			exit(-1);
		}
		
		for (int i = 0; i < *number_of_sections; i++) {
			if (read(fd, sh[i].sect_name, 6 * sizeof(char)) < 6) {
				fileSizeError();
			}
			sh[i].sect_name[6] = '\0';
			if (read(fd, &sh[i].sect_type, sizeof(char)) < 1) {
				fileSizeError();
			}
			switch(sh[i].sect_type) {
			case 19: 
				break;
			case 31:
				break;
			case 78: 
				break;
			case 45:
				break;
			case 17: 
				break;
			default:
				printf("ERROR\nwrong sect_types\n");
				exit(-1);
			}
			
			if(read(fd, &sh[i].sect_offset, sizeof(int)) < 4) {
				fileSizeError();	
			}
			
			if(read(fd, &sh[i].sect_size, sizeof(int)) < 4) {
				fileSizeError();
			}
		}
		return sh;	
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
	}
	else if (strcmp(cmd, "list") == 0) {
		listFunction(argc, argv);
	}
	else if (strcmp(cmd, "parse") == 0 || strcmp(argv[2], "parse") == 0) {
		if (argc != 3) {
			printf("ERROR\nInvalid number of arguments!\n");
			printf("Syntax: %s parse path=<file_path>\n", argv[0]);
			exit(-1);
		}
		char* path = NULL;
		for (int i = 1; i < argc; i++) { 
			if (strstr(argv[i], "path=") == argv[i]) {
				path = argv[i] + 5; 
			}
		}
		if (path == NULL || strlen(path) == 0) {
			printf("ERROR\ninvalid path\n");
			exit(-1);
		}
		int fd = open(path, O_RDONLY);
		if (fd < 0) {
			perror("ERROR\n");
			exit(-1);
		}
		
		unsigned short header_size, version;
		unsigned char number_of_sections;
		section_header* sh = parseSectionFile(fd, &header_size, &version, &number_of_sections);
		
		printf("SUCCESS\n");
		printf("version=%hu\n", version);
		printf("nr_sections=%d\n", number_of_sections);
		for (int i = 0; i < number_of_sections; i++) {
			printf("section%d: %s %d %d\n", i + 1, sh[i].sect_name, sh[i].sect_type, sh[i].sect_size);
		}
		free(sh);
		close(fd);
	}
	else if (strcmp(cmd, "extract") == 0) {
		if (argc != 5) {
			printf("ERROR\nInvalid number of arguments!");
			printf("Syntax: %s extract path=<file_path> section=<sect_nr> line=<line_nr>\n", argv[0]);
			exit(-1);
		}
		
		char* path = NULL;
		if (strstr(argv[2], "path=") == argv[2])
			path = argv[2] + 5;
		else {
			printf("ERROR\nInvalid syntax\n");
			exit(-1);
		}
		if (path == NULL || strlen(path) == 0) {
			printf("ERROR\ninvalid file\n");
			exit(-1);
		}
		
		int sect_nr;
		if (strstr(argv[3], "section=") == argv[3])
			sect_nr = atoi(argv[3] + 8);
		else {
			printf("ERROR\nInvalid syntax\n");
			exit(-1);
		}
		
		int line_nr;
		if (strstr(argv[4], "line=") == argv[4])
			line_nr = atoi(argv[4] + 5);
			
		int fd = open(path, O_RDONLY);
		if (fd < 0) {
			printf("ERROR\ninvalid file\n");
			exit(-1);
		}
		
		unsigned short header_size, version;
		unsigned char number_of_sections;
		section_header* sh = parseSectionFile(fd, &header_size, &version, &number_of_sections);
		if ( sect_nr> number_of_sections) {
			printf("ERROR\ninvalid section\n");
			exit(-1);
		}
		int current_line = 1;
		short endline = 0x0A0D, c;
		int section_final = sh[sect_nr - 1].sect_offset + sh[sect_nr - 1].sect_size - 1, section_start = sh[sect_nr - 1].sect_offset;
		lseek(fd, section_final - 1, SEEK_SET);
		int pos_start = section_final, pos_final = section_start - 1;
		for (int i = section_final - 2; i >= section_start; i--) {
			if (read(fd, &c, sizeof(short)) < 2) 
				fileSizeError();
			lseek(fd, -3, SEEK_CUR);
			if (c == endline) {
				current_line++;
				if (current_line == line_nr) {
					pos_start = i;
				}
				else if (current_line == line_nr + 1) {
					pos_final = i + 2;
					break; 
				}
			} 
		}
		if (current_line != line_nr && current_line != line_nr + 1) {
			printf("ERROR\ninvalid line\n");
			exit(-1);
		}
		printf("SUCCESS\n");
		char ch;
		lseek(fd, pos_start, SEEK_SET);	
		for (int i = pos_start; i >= pos_final; i--) {
			if (read(fd, &ch, sizeof(char)) < 1 )
				fileSizeError();
			printf("%c", ch);
			lseek(fd, -2, SEEK_CUR);
		}
		printf("\n");
		
	}
	return 0;
}
