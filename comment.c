#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define POSTFIX ".com"
#define POSTFIX_SIZE sizeof(POSTFIX)

#define READ_FLAG "-r"
#define WRITE_FLAG "-w"
#define APPEND_FLAG "-a"
#define DELETE_FLAG "-d"

void failure()
{
	chdir("..");
}

/*
 * The utility was writtern by Kovlaev Danil
 * It allows to comment any file/directory
 * MIT License
 */

typedef enum {READ, WRITE, APPEND, DELETE, NOP} Action;
typedef enum {SUCCESS, FAIL} ActionResult;

ActionResult execute_read(int fd)
{
	char buffer[128];
	int bytes_read;

	while ((bytes_read = read(fd, buffer, 128)) > 0)
	{
		write(STDOUT_FILENO, buffer, bytes_read);
	}

	return SUCCESS;
}
ActionResult execute_write(int fd, char *buffer, int size)
{
	if (ftruncate(fd, 0) == -1)
		return FAIL;
	if (write(fd, buffer, size) == -1)
		return FAIL;
	return SUCCESS;
}
ActionResult execute_append(int fd, char *buffer, int size)
{
	if (lseek(fd, 0, SEEK_END) == -1)
		return FAIL;
	if (write(fd, buffer, size) == -1)
		return FAIL;

	return SUCCESS;
}
ActionResult execute_delete(char *filename)
{
	if (remove(filename) == -1)
		return FAIL;
	return SUCCESS;
}

typedef struct
{
	char *filename;
	off_t file_size;
	Action action;
} Statement;

ActionResult execute_statement(Statement *statement)
{
	int name_len = strlen(statement -> filename);
	char comment_filename[255 + POSTFIX_SIZE + 1];
	strcpy(comment_filename, statement -> filename);
	strcpy(comment_filename + name_len, POSTFIX);
	comment_filename[name_len + POSTFIX_SIZE] = 0;
	
	int fd = open(comment_filename, O_CREAT | O_RDWR, 0644);
	if (fd == -1)
		return FAIL;	

	if (statement -> action == NOP)
	{
		struct stat st;
		stat(comment_filename, &st);
		statement -> action = st.st_size == 0 ? WRITE : READ;
	}

	char buffer[256];
	switch(statement -> action)
	{
		case READ: 	return execute_read(fd);
		case WRITE: 	
		{
			int bytes_read = read(STDIN_FILENO, buffer, 256);
			return execute_write(fd, buffer, bytes_read);
		}
		case APPEND: 	
		{
			int bytes_read = read(STDIN_FILENO, buffer, 256);
			return execute_append(fd, buffer, bytes_read);
		}
		case DELETE: 	return execute_delete(comment_filename);
		default: return FAIL;
	}

	return FAIL;	
}

ActionResult prepare_statement(Statement *statement, int argc, char **argv)
{
	if (argc > 3 || argc == 1)
		return FAIL;

	statement -> action = NOP;
	statement -> filename = NULL;

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (statement -> action != NOP)
				return FAIL;

			if (strcmp(argv[i], READ_FLAG) == 0)
			       	statement -> action = READ;
			else if (strcmp(argv[i], WRITE_FLAG) == 0)
				statement -> action = WRITE;
			else if (strcmp(argv[i], APPEND_FLAG) == 0)
				statement -> action = APPEND;
			else if (strcmp(argv[i], DELETE_FLAG) == 0)
				statement -> action = DELETE;
			else 
				return FAIL;	
		}
		else 
		{
			if (statement -> filename != NULL)
				return FAIL;
			
			int len = strlen(argv[i]);
			if (len > 255)
				return FAIL;
			
			statement -> filename = malloc(len + 1);
			strcpy(statement -> filename, argv[i]);
			statement -> filename[len] = 0;	
		}
	}

	if (statement -> filename == NULL)
		return FAIL;

	struct stat st;
	if (stat(statement -> filename, &st) == -1)
		return FAIL;

	return SUCCESS;
}

int main(int argc, char **argv)
{
	Statement statement;
	if (prepare_statement(&statement, argc, argv) == FAIL)
	{
		printf("prepare_statement failed\n");
		exit(EXIT_FAILURE);
	}

	// check if directory .comments exists, if it's not - create and enter one
	struct stat st = {0};
	if (stat(".comments", &st) == -1)
		mkdir(".comments", 0700);

	if (chdir(".comments") == -1)
	{
		perror("chdir");
		exit(EXIT_FAILURE);
	}

	atexit(failure);
       	ActionResult result = execute_statement(&statement);	
	if (result == FAIL)
	{
		printf("execution failed\n");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
