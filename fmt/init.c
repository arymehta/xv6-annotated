8600 // init: The initial user-level program
8601 
8602 #include "types.h"
8603 #include "stat.h"
8604 #include "user.h"
8605 #include "fcntl.h"
8606 
8607 char *argv[] = { "sh", 0 };
8608 
8609 int
8610 main(void)
8611 {
8612   int pid, wpid;
8613 
8614   if(open("console", O_RDWR) < 0){
8615     mknod("console", 1, 1);
8616     open("console", O_RDWR);
8617   }
8618   dup(0);  // stdout
8619   dup(0);  // stderr
8620 
8621   for(;;){
8622     printf(1, "init: starting sh\n");
8623     pid = fork();
8624     if(pid < 0){
8625       printf(1, "init: fork failed\n");
8626       exit();
8627     }
8628     if(pid == 0){
8629       exec("sh", argv);
8630       printf(1, "init: exec sh failed\n");
8631       exit();
8632     }
8633     while((wpid=wait()) >= 0 && wpid != pid)
8634       printf(1, "zombie!\n");
8635   }
8636 }
8637 
8638 
8639 
8640 
8641 
8642 
8643 
8644 
8645 
8646 
8647 
8648 
8649 
