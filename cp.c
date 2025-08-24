#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"


/* 
 * Supports:
 * 1) File to File copy
 * 2) File to directory
 * 3) Entire directory copy (no -r flag required)
 *
 * DOES NOT SUPPORT
 * -- More than 2 paths
 * NOTE: strcat function added to ulib.c even though its Read Only, because it was required to create paths
 */

void cp_file(char *src_file, char *dest_file)
{
	int fd1 = -1, fd2 = -1;
        fd1 = open(src_file, O_RDONLY);
        fd2 = open(dest_file, O_WRONLY | O_CREATE);

	printf(1, "Dest path: %s\n", dest_file);
        if(fd1 < 0 || fd2 < 0)
	{
		printf(1, "FD Source: %d\n", fd1);
		printf(1, "FD Dest: %d\n", fd2);
		 printf(2, "Cannot open files!\n");
                 if(fd1 != -1) close(fd1);
                 if(fd2 != -1) close(fd2);
                 return;
        }

	char ch;

	while((read(fd1, &ch, 1)))
		write(fd2, &ch ,1);

	close(fd1);
	close(fd2);
	return;
}

void cp_dir(char *src, char *dest)
{
	/*
	 * FROM fs.h
	 * // Directory is a file containing a sequence of dirent structures.
	#define DIRSIZ 14

	struct dirent {
  	ushort inum;
 	char name[DIRSIZ];
	};

	*/
	int fd;
	struct dirent dir;
	struct stat st;

	char srcPath[128], destPath[128];

	if((fd = open(src, 0)) < 0){
		printf(2, "cp: cannot open directory %s\n", src);
		return;
	}
	
	mkdir(dest);

	while(read(fd, &dir, sizeof(dir)) == sizeof(dir))
	{
		if(dir.inum == 0) continue;

		if(strcmp(dir.name, ".") == 0 || strcmp(dir.name, "..") == 0)
		       continue;
		
		strcpy(srcPath, src);
		strcat(srcPath, "/");
		strcat(srcPath, dir.name);

		strcpy(destPath, dest);
		strcat(destPath, "/");
		strcat(destPath, dir.name);

		if(stat(srcPath, &st) < 0)
		{
			printf(2, "cpdir: cannot stat\n");
			continue;
		}

		if(st.type == T_FILE)
			cp_file(srcPath, destPath);
		else if (st.type == T_DIR){
			mkdir(destPath);
			cp_dir(srcPath, destPath);
		}
	}
	return;
}

void cp(char *src, char *dest)
{

	struct stat st_src, st_dest;
	if(stat(src, &st_src) < 0)
	{
		printf(2, "cp: cannot create %s: Does not exist!\n", src);
		return;
	}
	if(stat(dest, &st_dest) < 0)
	{
		printf(2, "cp: cannot create %s: Does not exist!\n", dest);
		return;
	}

	
        /*
         * Case1 : src is file dest is file --> simple copy
         * Case2: src is file dest is dir --> create new file dirname/file1.name, and simple copy
         * Case3: src is dir, dest is dir (need -r parameter) --> recursive copy
         * Case4: src is dir, dest is file --> error (cannot open dir)
         */

	/* FROM STAT.H
	#define T_DIR  1   // Directory
	#define T_FILE 2   // File
	#define T_DEV  3   // Device
	*/


	if(st_src.type == T_FILE && st_dest.type == T_FILE)
	{
		
		cp_file(src, dest);
		return;
	}
	
	else if(st_src.type == T_FILE && st_dest.type == T_DIR)
	{
		char destPath[128];

		strcpy(destPath, dest);
		strcat(destPath, "/");
		strcat(destPath, src);
		printf(1, "Destpath in if: %s\n", destPath);	
                cp_file(src, destPath);
		return;
	}

	else if(st_src.type == T_DIR && st_dest.type == T_DIR)
	{
		char newDest[128];
		strcpy(newDest, dest);
		strcat(newDest, "/");

		char *srcdirname = src;
		for(char *p = src; *p; p++)
		{
			if(*p == '/') srcdirname = p + 1;
		}

		strcat(newDest, srcdirname);
		mkdir(newDest);
		cp_dir(src, newDest);
		return;	
			
	}

	else if(st_src.type == T_DIR && st_dest.type == T_FILE)
	{
		printf(2, "Cant copy directory into a file!\n");	
	}
	
	return;
}


int main(int argc, char *argv[])
{
	if(argc != 3) 
	{
		printf(2, "Wrong input format!\n");
		exit();
	}
	cp(argv[1], argv[2]);
	exit();
}
