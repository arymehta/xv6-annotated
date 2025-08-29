#include "types.h"
#include "stat.h"
#include "user.h"
//#include "file.h"
#include "fcntl.h"


   int main()
   {
          int fd1 = open("temp.txt", O_CREATE | O_WRONLY);
          write(fd1, "hello", 5);
   
          lseek(fd1, -1, 1);
          write(fd1, "ly", 2);
	  exit();
  }
