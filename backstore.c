#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>

// --- To Do --- //

// 1. Finish settings



// DONT be a DICK! This is my FIRST program I've made in C and there are many things I'd like to add to it! 
// If you feel there is an essential feature missing, please let me know on my github at 


// --- Settings --- //

bool archiveDuplicates;

// --- End Settings --- //

// --- Start Global Vars & Functions --- //

char* setNameBuf;
char* setValBuf;
char *temp;

// Flags
void info(); 
int help();
int createProfile(char *argList[256]);
int changeSetting(char* argList[256]);
int showProfiles();
int load(char* name);
int delProfile(char* name);
int saveConfig(char* servName);

// Parser
void parser(int argc, char *argv[]);
bool isFlag(int argc, char *argv[], int i);
bool isNullArg(int argc, char *argv[], int i);
bool flagEquals(char* flag, char* x, char* y, char* z);
void dealFlag(char *flag, char* argList[256]);

// Prep & Utility 
int createDir(char* name);
int createTxt(char* servName, char* confFilePath);
char* readTxt(char* servName, int i);
void copyConfig();
char *profNameBuilder (char *service, int i, char* specName);
bool isDiff(char* servName, char* fileName);
int extractNumber(const char *fileName);
int findDif(char* servName, char* fileName);
char* getConfName(char* name);
bool profExists(char* name);

// Settings
void getSetName(char buf[256]);
void createSettings();
char* readSetting(char buf[256], int choice);

// --- End Global Vars & Functions --- //



// --- MAIN CODE --- //



int main (int argc, char *argv[]) 
{
	createSettings();

	if (argc == 1) 
	{
		info();
	}
	if (argc > 1)
	{	
		parser(argc, argv);
	}
}

void dealFlag(char* flag, char* argList[256]) { // Post Parse Flag Processing

	if ((flagEquals(flag, "-c", "--c", "--createprofile")) && argList != NULL) {
		if(createProfile(argList) == 1) {
			fprintf(stderr, "An error has occured in creating a profile\n");
		}

	} else if (flagEquals(flag, "--h", "-h", "help")) {
		if (help() == 1) {
			fprintf(stderr, "An error has occured in printing help, what the!?\n");
		}

	} else if (flagEquals(flag, "-s", "--s", "--changesetting")) {
		if (changeSetting(argList) == 1) {
			fprintf(stderr, "Error - Changing Setting... Just use the config man :(\n");
		}

	} else if (flagEquals(flag, "-sp", "--sp", "--showprofiles")) {
		if (showProfiles() == 1) {
			fprintf(stderr, "Error - Showing Profiles\n");
		}

	} else if (flagEquals(flag, "-sc", "--sc", "--saveconfig")) {
		if (saveConfig(argList[0]) == 1) {
			fprintf(stderr, "Error - Saving Config\n");
		}

	} else if (flagEquals(flag, "-dp", "--dp", "--deleteprofile")) {
		if (delProfile(argList[0]) == 1) {
			fprintf(stderr, "Error - Deleting Profile\n");
		}

	} else if (flagEquals(flag, "-cs", "--cs", "--createsetting")) {
		createSettings();
	
	} else if (flagEquals(flag, "-lc", "--lc", "--deleteprofile")) {
		if (load(argList[0]) == 1) {
			fprintf(stderr, "Error - Problem Loading Profile\n");
		}

	} else {
		help();
	}
}




// --- PREP & FUNCTION --- //




int createDir(char* name) { // Creates ../profile/[service]/ & ../profile/[service]/configs/ Directories

	if (mkdir(profNameBuilder(name, 1, NULL), 0777) == 0) {
		printf("Created profile dir success\n");
	}
	else {
		perror("Error creating profile directory");
		return 1;
	}

	if (mkdir(profNameBuilder(name, 3, NULL), 0777) == 0) {
		printf("Created conf dir success\n");
	}
	else {
		perror("Error creating ../profiles/configs/ directory");
		return 1;
	}

	return 0;
}


int findDif(char* servName, char* fileName) { // Finds and Prints the difference between 2 files, line by line

	char curBuf[1024];
	char altBuf[1024];
	bool isDif = false;

	int line = 0;

	FILE *curConf = fopen(readTxt(servName, 1), "r");

	FILE *altConf = fopen(profNameBuilder(servName, 4, fileName), "r");

	if (curConf == NULL || altConf == NULL) {
        perror("Error opening files");
    }

	while (fgets(curBuf, sizeof(curBuf), curConf) != NULL && fgets(altBuf, sizeof(altBuf), altConf) != NULL) {

		if (line == 0 && (isDiff(servName, fileName))) {
			printf("----- %s -----\n", fileName);
		}

		if (strcmp(curBuf, altBuf) != 0) {
			printf("Line %d: %s\n", line, altBuf);
			isDif = true;
		}

		line++;
 	}

	fclose(altConf);
	fclose(curConf);
}

bool isDiff(char* servName, char* fileName) { // Returns wether 2 files are the different [true] or the same [false]
	char res[256];
	strcpy(res, profNameBuilder(servName, 4, fileName));
	//printf("res: %s\n", res);

	FILE *comp1 = fopen(res, "r");
	char comp1Buf[1024];
	
	FILE *comp2 = fopen(readTxt(servName, 1), "r");
	char comp2Buf[1024];

	bool isDif = false;

	while (fgets(comp1Buf, sizeof(comp1Buf), comp1) != NULL && fgets(comp2Buf, sizeof(comp2Buf), comp2) != NULL) {

		if(strcmp(comp1Buf, comp2Buf) != 0) {
			return true;
		}
	}

	return false;

}

int createTxt(char* servName, char* confFilePath) { // Creates the [service]_file_loc.conf file in ../profiles/[service]/ and Writes the main service config file path to it

	FILE* file = fopen(profNameBuilder(servName, 2, NULL), "w");

	fprintf(file, "%s", confFilePath);

	fclose(file);

	return 0;
} 

char* readTxt(char* servName, int i) { // Reads main service config file path [1] or just the main config name [2]

	static char res[256];
	char backBuf[256];
	static char tempBuf[256];

	bool found = false;

	int ind = 0;

	FILE* file = fopen(profNameBuilder(servName, 2, NULL), "r"); 

	fgets(res, sizeof(res), file);

	if (i == 1) {
		return res;
	}

	for (int i = (strlen(res) - 1); i > 0; i--) {
		if (res[i] == '/') {
			found = true;
		}
		if (!found) {
			backBuf[ind] = res[i];
			backBuf[ind + 1] = '\0';
		}
		ind++;
	}

	ind = 0;

	for (int i = (strlen(backBuf) - 1); i >= 0; i--) {
		tempBuf[ind] = backBuf[i];
		tempBuf[ind + 1] = '\0';
		ind++;
	}
	
	if (i == 2) {
		return tempBuf;
	}

	fclose(file);
}

void copyConfig(char* servName) { // Copies the original config contents to ../profiles/[service]/original.conf when '-c' is run
	char res[256];

	strcpy(res, profNameBuilder(servName, 1, NULL));
	strcat(res, "original.conf");

	FILE *orgCpy = fopen(res, "w");
	printf("goop mode: %s\n", profNameBuilder(servName, 1, "original.conf"));
	FILE *org = fopen(readTxt(servName, 1), "r");

	char orgBuf[1024];

	printf("I made it!!\n");

	while (fgets(orgBuf, sizeof(orgBuf), org) != NULL) {
		fprintf(orgCpy, "%s", orgBuf);
		printf("%s\n", orgBuf);
	}

	fclose(orgCpy);
	fclose(org);
}

char *profNameBuilder (char *service, int i, char *specName) { // Used to construct path strings  
	char* str = malloc(1024);

	strcpy(str, "../profiles/");
	strcat(str, service);
	strcat(str, "/");

	if (i == 1) { // ../profiles/[service]/ [1]
		return str;

	} else if (i == 2) { // ../profiles/[service]/[special-name] [2] ../profiles/[service]/[service]_file_loc.conf [2] when specName == NULL

		if (specName != NULL) {
			strcat(str, specName);
		} else {
			strcat(str, service);
			strcat(str, "_file_loc.conf");
		}

		return str;
	} else if (i == 3) { // ../profiles/[service]/configs/ [3]
		strcat(str, "configs/"); 
		return str;

	} else if (i == 4) { // ../profiles/[service]/configs/[specName] [4] ../profiles/[service]/configs/[service]_file_loc.conf [4] when specName == NULL

		strcat(str, "configs/");
		if (specName == NULL) {
			strcat(str, service);
			strcat(str, "_file_loc.conf");
		} else {
			strcat(str, specName);
		}
		return str;
	}
}

bool profExists(char* name) { // Returns true if the ../profiles/[service] exists
	DIR *dir = opendir(profNameBuilder(name, 1, NULL));

	if (dir != NULL) {
		closedir(dir);
		return true;
	}
	else {
		return false;
	}
}




// --- ARG PARSER --- //




bool flagEquals(char* flag, char* x, char* y, char* z) { // Returns true if 'flag' string equals any of the 3 'x,y,z' strings, false otherwise
	if ((strcmp(flag, x) == 0) || (strcmp(flag, y) == 0) || (strcmp(flag, z) == 0)) {
		return true;
	}

	return false;
}

bool isFlag(int argc, char *argv[], int i) { // Returns true if an arg starts with either '--' or '-'
	char *buf;
	char *buf2;

	buf = argv[i];
	buf2 = argv[i+1];

	if ((argv[i] == NULL) || (strcmp(argv[i], "\0") == 0)) {
		return false;
	}

	char buffer[3];
	sprintf(buffer, "%c%c", buf[0], buf[1]);
	if ((buf[0] == '-') || (strcmp(buffer, "--")) == 0) {
		return true;
	}
	else {
		return false;
	}
}

bool isNullArg(int argc, char *argv[], int i) { // Returns true if arg is "\0" or NULL
	if ((argv[i] == NULL) || (strcmp(argv[i], "\0") == 0)) {
		return true;
	}
	return false;
}

void parser(int argc, char *argv[]) // Used to create list of valid args for given flag and send to 'dealFlag'
{
	int size = 0;
	char* strList[256];

	for (int i = 1; i < argc; i++) {

		if (isFlag(argc, argv, i) && (isFlag(argc, argv, i + 1) || isNullArg(argc, argv, i + 1))) { //Deal with lonely flag when next arg is flag
		
			dealFlag(argv[i], strList);

		} else if (!isFlag(argc, argv, i) && (isFlag(argc, argv, i + 1) || isNullArg(argc, argv, i + 1))) { //Add non-flag arg to list and then deal with it

			strList[size] = argv[i];
			size++;
			dealFlag(argv[i - size], strList);
			*strList = NULL;
			size = 0;

		} else if (!isFlag(argc, argv, i) && !isNullArg(argc, argv, i)) {

			strList[size] = argv[i];
			size++;

		} else if (isFlag(argc, argv, i)) {

		}
	}
}




// --- FLAGS --- //




void info() // Tells you how to use help, is called if no args are given
{
	printf("If you need help, use the -h or --help flag\r\n");
}

int help() // Tells user how to use each flag
{
	printf("Type in profile to creat\r\n");
}

int createProfile(char *argList[256]) { // Runs createDir, createTxt, and copyConfig. It creates the profile 

	if (argList[0] != NULL) {
		createDir(argList[0]);

		if (argList[1] != NULL) {
			createTxt(argList[0], argList[1]);
			copyConfig(argList[0]);
			return 0;
		}
	}
	return 1;
}

int delProfile(char* name) { // Deletes the entire profile including configs

	DIR *dir = opendir("../profiles/");

	if(dir == NULL) {
		perror("Error deleting profile");
	}

    struct dirent *ent;

	char input;
	bool found = false;

	char command[1024];
	snprintf(command, sizeof(command), "rm -r %s", profNameBuilder(name, 1, NULL));


    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) { 
			if (strcmp(ent->d_name, name) == 0) {

				printf("Are you sure you would like to delete the %s profile? It will delete ALL archived configs [y or n] ");
				found = true;
				scanf("%c", &input);
				printf("\n");

				if (input == 'y'){
					
					if (system(command) == 0) {
						printf("Success - Deleted profile %s\n", name);
					} else {
						fprintf(stderr, "Error - Problem deleting profile %s\n", name);
					}
				} else if (input == 'n') {
					printf("OK - I will not delete profile %s\n", name);
				}
			}
        }
    }

	if (!found) {
		fprintf(stderr, "Woops - Could not find profile %s\n", name);
	}

    closedir(dir);
	return 0;
}

int showProfiles() { // Shows which service config profiles are in the ../profiles/ directory
	DIR *dir = opendir("../profiles/");

	if (dir == NULL) {
		perror("Show Profile Error");
		return 0;
	}

	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			printf("%s\n", entry->d_name);
		}
	}

	closedir(dir);

	return 0;
}

int saveConfig(char* servName) { // Saves current running service config to ../profiles/[service]/configs/ with a number at the beginning

	struct dirent *ent;
	struct dirent *ent2;

	bool isDif = true;

	DIR *dir = opendir(profNameBuilder(servName, 3, NULL));
	DIR *confDir = opendir(profNameBuilder(servName, 3, NULL));
	FILE *file = fopen(readTxt(servName, 1), "r");

	if(dir == NULL || confDir == NULL || file == NULL) {
		perror("Save Config Error");
	}


	char numCh[2];
	int num = 0;
	int tmpNum = 0;

	while ((ent = readdir(dir)) != NULL) {

		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
			
			printf("%c\n", ent->d_name[0]);

			numCh[0] = ent->d_name[0];
			numCh[1] = '\0';
			sscanf(numCh, "%d", &tmpNum);

			if (tmpNum > num) {
				num = tmpNum;
			}
		}
	}

	char fileName[256];

	sprintf(fileName, "%d", (num+1));
	strcat(fileName, "_");
	strcat(fileName, readTxt(servName, 2));

	while ((ent2 = readdir(confDir)) != NULL) {

		if (strcmp(ent2->d_name, ".") != 0 && strcmp(ent2->d_name, "..") != 0) {
			
			if (!isDiff(servName, ent2->d_name) && archiveDuplicates == false) {
				fprintf(stderr, "Error - Cannot add duplicate config for %s, fix this in backstore.conf\n", servName);
				isDif = false;
			}
		}
	}

	if (isDiff) {
		FILE *new = fopen(profNameBuilder(servName, 4, fileName), "w+");

		char curBuf[1024];

		while (fgets(curBuf, sizeof(curBuf), file) != NULL) {
			fprintf(new, "%s", curBuf);
		}
		fclose(new);
	}

	closedir(dir); closedir(confDir); fclose(file);
}

int load(char* name) { // Shows the user all the differences in archived config files compared to the running one and allows them to load an archived config of their choosing

	if (!profExists(name)) {
		fprintf(stderr, "Error - Profile does not exist\n");
		return 1;
	}

	FILE *curConf = fopen(readTxt(name, 1), "r");
	
	DIR *dir = opendir(profNameBuilder(name, 3, NULL));

	if (curConf == NULL || dir == NULL) {
		perror("Load config error");
	}

	int usrInput;
	char usrString[20];

	int i = 0;
	struct dirent *ent;

	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {

			findDif(name, ent->d_name);
			i++;
		}
	}

	printf("Which config archive would you like to load? ID: ");
	scanf("%d", &usrInput);
	sprintf(usrString,"%d", usrInput);
	printf("\n");

	char res[256];

	char* read = readTxt(name, 2);
	
	strcpy(res, usrString);
	strcat(res, "_");
	strcat(res, read);

	char chosenBuf[1024];
	char curBuf [1024];

	FILE *chosen = fopen(profNameBuilder(name, 4, res), "r"); //Chosen archive
	FILE *curRead = fopen(readTxt(name, 1), "r"); //Read current config
	FILE *curWrite = fopen(readTxt(name, 1), "w"); //Write to current config
 	FILE *confBuffer = fopen(profNameBuilder(name, 2, "buffer.conf"), "w"); //Write old config to buffer file

	if(chosen == NULL || curRead == NULL || curWrite == NULL || confBuffer == NULL) {
		perror("Load config error");
	}

	while (fgets(curBuf, sizeof(curBuf), curRead) != NULL) {
		fprintf(confBuffer, "%s", curBuf);
	}

	while (fgets(chosenBuf, sizeof(chosenBuf), chosen) != NULL) {
		fprintf(curWrite, "%s", chosenBuf);
	}

	closedir(dir);
	fclose(curConf); fclose(curRead); fclose(confBuffer); fclose(curWrite); fclose(chosen);

	return 0;
}





// --- SETTINGS PARSER --- //




int changeSetting(char* argList[256]) { // Not done

	char buf[256];
	int line = 0;
	char res[256];

	strcpy(res, argList[0]);
	strcat(res, ":");
	strcat(res, argList[1]);

	FILE *file = fopen("set.conf", "r");

	FILE *tmp = fopen("temp.conf", "w");

	while (fgets(buf, sizeof(buf), file) != NULL) {
		line++;

		if (strstr(setNameBuf, argList[0]) && !strstr(setValBuf, argList[1])) {
			fprintf(tmp, "%s\n", res);
		} 
		else {
			fputs(buf, tmp);
		}
		
		printf("buf: %s", buf);
	}

	fclose(file); fclose(tmp);

	return 0;
}

void createSettings() { // Creates 'backstore.conf' if it does not exist, writes default settings

	char buf[256];

	const char *filename = "../backstore.conf";

	if (access(filename, 0) != 0) {
		printf("backstore.conf does not exist, creating it...\n");

		FILE *file = fopen(filename, "w");

		fprintf(file, "archive-duplicates:no\n");
	}

	FILE *file = fopen("../backstore.conf", "r");

	while (fgets(buf, sizeof(buf), file) != NULL) {
		char* val = readSetting(buf, 1);
		char* name = readSetting(buf, 2);

		if (strcmp(name, "archive-duplicates") == 0) {
			if (strcmp(val, "no\n") == 0) {
				archiveDuplicates = false;

			} else if (strcmp(val, "yes\n") == 0) {
				archiveDuplicates = true;
			}
		}
	}

	fclose(file);
}

char* readSetting(char buf[256], int choice) { // Returns either a value [1] or setting name [2] when given the buffer from reading the 'backstore.conf' file line by line

	static char name[128];
	int nameSize = 0;
	static char val[128];
	int valSize = 0;
	bool foundCol = false;

	for (int i = 0; i < strlen(buf); i++) {
		if (buf[i] == ':') {
			foundCol = true;
		}
		else if ((!foundCol) && (buf[i] != ':')) {
			name[nameSize] = buf[i];
			nameSize++;
		}
		else if (foundCol) {
			val[valSize] = buf[i];
			valSize++;
		}
	}
	if (choice == 1) {
		return val;
	} else if (choice == 2) {
		return name;
	}
}

