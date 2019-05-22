/*
 * Andrew Yang
 * V00878595
 * CSC 360 - Operating Systems
 * Assignment 1 - kapish
 * 1/31/19
 */

/*
 * References
 * Author: Stephen Brennan
 * Title: Tutorial - Write a Shell in C
 * Date: 1/31/19
 * Availability: https://brennan.io/2015/01/16/write-a-shell-in-c/
 */

#define _DEFAULT_SOURCE

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 512
#define TOKEN_DELIM " \t\r\n\a"
#define TOKEN_BUFFER 64
#define _GNU_SOURCE
#define HISTORY_SIZE 512

int setenv_ks(char **args);
int unsetenv_ks(char **args);
int command_cd(char **command_dir);
int exit_ks(char **args);
int launch_prog(char **args);
//int print_history(char **history_buffer);
int kapishrc(char **args);
//void store_history(char **args);
int execute(char **args);
char** tokenize_line(char* line);


//pid_t pid_var;

char *builtins[] = {
	"setenv",
	"unsetenv",
	"cd",
	"exit",
	"kapishrc"
};

int (*builtin_func[]) (char **) = {
	&setenv_ks,
	&unsetenv_ks,
	&command_cd,
	&exit_ks,
	&kapishrc
};

int setenv_ks(char **args){
	char* stenv = "setenv";

	if(setenv(args[1], args[2], 1) == -1){
		fprintf(stderr, "setenv error");
		exit(EXIT_FAILURE);
	}
	if(args[2] == NULL){
		printf("%s %s\n", stenv, args[1]);
	}else{
		printf("%s %s %s\n", stenv, args[1], args[2]);
	}
	return 1;
}

int unsetenv_ks(char **args){
	if(unsetenv(args[1]) == -1){
		fprintf(stderr, "unsetenv error");
		exit(EXIT_FAILURE);
	}
	return 1;
}

int command_cd(char **command_dir){
	if(command_dir[1] == NULL){
		fprintf(stderr, "change directory error");
	}else{
		if(chdir(command_dir[1]) != 0){
			perror("chdir error");
		}
	}
	return 1;
}

int exit_ks(char **args){
	return 0;
}

int kapishrc(char **args){
	char * buffer = malloc(sizeof(char) * 64);
	char **tokenize;
	char * path;
	long length;

	path = getenv("HOME");
	FILE *file = fopen(strcat(path, "/.kapishrc"), "r");

	if(file == NULL){
		printf("Could not load file\n");
		return 1;
	}
	while(fgets(buffer, 512, file) != NULL){
		tokenize = tokenize_line(buffer);
		execute(tokenize);
	}

	fclose(file);
	return 1;
}

/*int print_history(char **args){
	int i = 0;
	while(history[i] != NULL){
		printf("%s", history[i++]);
	}

	return 1;
}

void store_history(char **args){
	int i = 0;
	int buffsize = HISTORY_SIZE;
	history = malloc(sizeof(char*) * HISTORY_SIZE);

	if(history_count < buffsize){
		while(args[i] != NULL){
			history[history_count++] = strdup(args[i]);
		}
		history[history_count++] = "\n";
	}else{
		buffsize += HISTORY_SIZE;
		history = realloc(history, buffsize);
		if(!history){
			fprintf(stderr, "History buffer allocation error\n");
			exit(EXIT_FAILURE);
		}
	}
}*/

/*void sig_handler(int mysignal){
	if(pid_var == 0){
		kill(pid_var, SIGTERM);
	}
}*/

char* read_line(void){
	int buffsize = BUFFER_SIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * buffsize);
	int c;
	char **args;

	if(!buffer){
		fprintf(stderr, "buffer allocation error\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		c = getchar();

		if(c == EOF || c == '\n'){
			exit_ks(args);
			buffer[position] = '\0';
			return buffer;
		}else{
			buffer[position] = c;
		}
		position++;

		if(position >= buffsize){
			buffsize += BUFFER_SIZE;
			buffer = realloc(buffer, buffsize);
			if(!buffer){
				fprintf(stderr, "buffer allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char** tokenize_line(char* line){
	int size_buff = TOKEN_BUFFER;
	char** token_buffer = malloc(sizeof(char*) * size_buff);
	int position = 0;
	char* token;

	if(!token_buffer){
		//error
		fprintf(stderr, "token buffer allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, TOKEN_DELIM);
	while(token != NULL){
		token_buffer[position] = token;
		position++;

		if(position >= size_buff){
			size_buff += TOKEN_BUFFER;
			token_buffer = realloc(token_buffer, size_buff);
			if(!token_buffer){
				//exit program
				fprintf(stderr, "buffer allocation error");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, TOKEN_DELIM);
	}
	token_buffer[position] = NULL;
	return token_buffer;
}

int execute(char **args){
	int i;

	if(args[0] == NULL){
		return 1;
	}

	for(i = 0; i< 5; i++){
		if(strcmp(args[0], builtins[i]) == 0){
			//printf("checking: %d\n", i);
			return (*builtin_func[i])(args);
		}
	}

	return launch_prog(args);
}

int launch_prog(char **args){
	pid_t pid, cpid;
	int status;

	pid = fork();
	if(pid == 0){
		if(execvp(args[0], args) == -1){
			perror("kapish");
		}
		/*if(setpgid(pid, pid) == -1){
			perror("setpid");
		}*/
		//pid_var = pid;
		//signal(SIGINT, SIG_DFL);
		//kill(pid, SIGTERM);
		//return 1;
		exit(EXIT_FAILURE);
		// child
	} else if(pid < 0){
		perror("kapish");
	} else{
		do{
			/*if(setpgid(pid, pid) == -1){
				perror("setpid");
			}*/
			signal(SIGINT, SIG_IGN);
			cpid = waitpid(pid, &status, WUNTRACED);
			signal(SIGINT, SIG_DFL);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));	
	}
	return 1;
}

void kapish_loop(void){
	char* line;
	int check_end;
	char** args;
	int status;
	char** history;

	do{
		printf("? ");
		check_end = feof(stdin);
		if(check_end != 0){
			exit_ks(args);
		}
		line = read_line();
		args = tokenize_line(line);
		//store_history(args);
		status = execute(args);

		free(line);
		free(args);
	} while (status);
	free(history);
}

int main(int argc, char **argv){
	kapish_loop();

	return EXIT_SUCCESS;
}



