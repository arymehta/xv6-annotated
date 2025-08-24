#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

#define LINESIZE 2048
#define EOF -1
int readline(int fd, char *line)
{
	char ch;
	char *ptr = line;
	int n = 0;
	
	while(1)
	{
		int r = read(fd, &ch, 1);
		if(r < 1) break;
		if(ch == '\n') break;
		*ptr++ = ch;
		n++;
	}
	*ptr = '\0';
	return ++n;
}

int head(int argvalue, char flag, char *filename)
{
	int fd = open(filename, O_RDONLY);
	if(fd == -1)
	{
		printf(2, "Cannot open file!\n");
		return -1;
	}

	if(flag == 'n')
	{
		// print argvalue lines!
		char *line = malloc(sizeof(char) * LINESIZE);

		while(argvalue-- > 0){
			int n = readline(fd, line);
			if(n <= 0) break;
			printf(1, "%s\n", line);
		}
		
	}

	else if(flag == 'c')
	{
		// print argvalue bytes
		char ch = '\0';
		while(argvalue--)
		{
			int n = read(fd, &ch, 1);
			if(n <= 0) break;
			printf(1, "%c", ch);
		}
		printf(1, "\n");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int argvalue = 10;
	char flag = 'n';
	char *filename;
       	// default will print first 10 lines

	for(int i = 1; i < argc; i++)
	{
		
		if(strcmp(argv[i], "-n") == 0 && i + 1 < argc)
		{
			argvalue = atoi(argv[i + 1]);
			if(argvalue == -1){
				printf(2, "Invalid number of lines!");
				exit();
			}
			i++;
			flag = 'n';
		}

		else if(strcmp(argv[i], "-c") == 0 && i + 1 < argc)
		{
			argvalue = atoi(argv[i + 1]);
			if(argvalue == -1){
				printf(2, "Invalid number of bytes!\n");
				exit();
			}
			i++;
			flag = 'c';
		}
		
		else
	 		filename = argv[i];
	}
	
//	printf(1, "Filename: %s\n", filename);
	head(argvalue, flag, filename);
	exit();
}
