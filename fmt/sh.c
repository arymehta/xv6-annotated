8650 // Shell.
8651 
8652 #include "types.h"
8653 #include "user.h"
8654 #include "fcntl.h"
8655 
8656 // Parsed command representation
8657 #define EXEC  1
8658 #define REDIR 2
8659 #define PIPE  3
8660 #define LIST  4
8661 #define BACK  5
8662 
8663 #define MAXARGS 10
8664 
8665 struct cmd {
8666   int type;
8667 };
8668 
8669 struct execcmd {
8670   int type;
8671   char *argv[MAXARGS];
8672   char *eargv[MAXARGS];
8673 };
8674 
8675 struct redircmd {
8676   int type;
8677   struct cmd *cmd;
8678   char *file;
8679   char *efile;
8680   int mode;
8681   int fd;
8682 };
8683 
8684 struct pipecmd {
8685   int type;
8686   struct cmd *left;
8687   struct cmd *right;
8688 };
8689 
8690 struct listcmd {
8691   int type;
8692   struct cmd *left;
8693   struct cmd *right;
8694 };
8695 
8696 struct backcmd {
8697   int type;
8698   struct cmd *cmd;
8699 };
8700 int fork1(void);  // Fork but panics on failure.
8701 void panic(char*);
8702 struct cmd *parsecmd(char*);
8703 
8704 // Execute cmd.  Never returns.
8705 void
8706 runcmd(struct cmd *cmd)
8707 {
8708   int p[2];
8709   struct backcmd *bcmd;
8710   struct execcmd *ecmd;
8711   struct listcmd *lcmd;
8712   struct pipecmd *pcmd;
8713   struct redircmd *rcmd;
8714 
8715   if(cmd == 0)
8716     exit();
8717 
8718   switch(cmd->type){
8719   default:
8720     panic("runcmd");
8721 
8722   case EXEC:
8723     ecmd = (struct execcmd*)cmd;
8724     if(ecmd->argv[0] == 0)
8725       exit();
8726     exec(ecmd->argv[0], ecmd->argv);
8727     printf(2, "exec %s failed\n", ecmd->argv[0]);
8728     break;
8729 
8730   case REDIR:
8731     rcmd = (struct redircmd*)cmd;
8732     close(rcmd->fd);
8733     if(open(rcmd->file, rcmd->mode) < 0){
8734       printf(2, "open %s failed\n", rcmd->file);
8735       exit();
8736     }
8737     runcmd(rcmd->cmd);
8738     break;
8739 
8740   case LIST:
8741     lcmd = (struct listcmd*)cmd;
8742     if(fork1() == 0)
8743       runcmd(lcmd->left);
8744     wait();
8745     runcmd(lcmd->right);
8746     break;
8747 
8748 
8749 
8750   case PIPE:
8751     pcmd = (struct pipecmd*)cmd;
8752     if(pipe(p) < 0)
8753       panic("pipe");
8754     if(fork1() == 0){
8755       close(1);
8756       dup(p[1]);
8757       close(p[0]);
8758       close(p[1]);
8759       runcmd(pcmd->left);
8760     }
8761     if(fork1() == 0){
8762       close(0);
8763       dup(p[0]);
8764       close(p[0]);
8765       close(p[1]);
8766       runcmd(pcmd->right);
8767     }
8768     close(p[0]);
8769     close(p[1]);
8770     wait();
8771     wait();
8772     break;
8773 
8774   case BACK:
8775     bcmd = (struct backcmd*)cmd;
8776     if(fork1() == 0)
8777       runcmd(bcmd->cmd);
8778     break;
8779   }
8780   exit();
8781 }
8782 
8783 int
8784 getcmd(char *buf, int nbuf)
8785 {
8786   printf(2, "$ ");
8787   memset(buf, 0, nbuf);
8788   gets(buf, nbuf);
8789   if(buf[0] == 0) // EOF
8790     return -1;
8791   return 0;
8792 }
8793 
8794 
8795 
8796 
8797 
8798 
8799 
8800 int
8801 main(void)
8802 {
8803   static char buf[100];
8804   int fd;
8805 
8806   // Ensure that three file descriptors are open.
8807   while((fd = open("console", O_RDWR)) >= 0){
8808     if(fd >= 3){
8809       close(fd);
8810       break;
8811     }
8812   }
8813 
8814   // Read and run input commands.
8815   while(getcmd(buf, sizeof(buf)) >= 0){
8816     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
8817       // Chdir must be called by the parent, not the child.
8818       buf[strlen(buf)-1] = 0;  // chop \n
8819       if(chdir(buf+3) < 0)
8820         printf(2, "cannot cd %s\n", buf+3);
8821       continue;
8822     }
8823     if(fork1() == 0)
8824       runcmd(parsecmd(buf));
8825     wait();
8826   }
8827   exit();
8828 }
8829 
8830 void
8831 panic(char *s)
8832 {
8833   printf(2, "%s\n", s);
8834   exit();
8835 }
8836 
8837 int
8838 fork1(void)
8839 {
8840   int pid;
8841 
8842   pid = fork();
8843   if(pid == -1)
8844     panic("fork");
8845   return pid;
8846 }
8847 
8848 
8849 
8850 // Constructors
8851 
8852 struct cmd*
8853 execcmd(void)
8854 {
8855   struct execcmd *cmd;
8856 
8857   cmd = malloc(sizeof(*cmd));
8858   memset(cmd, 0, sizeof(*cmd));
8859   cmd->type = EXEC;
8860   return (struct cmd*)cmd;
8861 }
8862 
8863 struct cmd*
8864 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
8865 {
8866   struct redircmd *cmd;
8867 
8868   cmd = malloc(sizeof(*cmd));
8869   memset(cmd, 0, sizeof(*cmd));
8870   cmd->type = REDIR;
8871   cmd->cmd = subcmd;
8872   cmd->file = file;
8873   cmd->efile = efile;
8874   cmd->mode = mode;
8875   cmd->fd = fd;
8876   return (struct cmd*)cmd;
8877 }
8878 
8879 struct cmd*
8880 pipecmd(struct cmd *left, struct cmd *right)
8881 {
8882   struct pipecmd *cmd;
8883 
8884   cmd = malloc(sizeof(*cmd));
8885   memset(cmd, 0, sizeof(*cmd));
8886   cmd->type = PIPE;
8887   cmd->left = left;
8888   cmd->right = right;
8889   return (struct cmd*)cmd;
8890 }
8891 
8892 
8893 
8894 
8895 
8896 
8897 
8898 
8899 
8900 struct cmd*
8901 listcmd(struct cmd *left, struct cmd *right)
8902 {
8903   struct listcmd *cmd;
8904 
8905   cmd = malloc(sizeof(*cmd));
8906   memset(cmd, 0, sizeof(*cmd));
8907   cmd->type = LIST;
8908   cmd->left = left;
8909   cmd->right = right;
8910   return (struct cmd*)cmd;
8911 }
8912 
8913 struct cmd*
8914 backcmd(struct cmd *subcmd)
8915 {
8916   struct backcmd *cmd;
8917 
8918   cmd = malloc(sizeof(*cmd));
8919   memset(cmd, 0, sizeof(*cmd));
8920   cmd->type = BACK;
8921   cmd->cmd = subcmd;
8922   return (struct cmd*)cmd;
8923 }
8924 
8925 
8926 
8927 
8928 
8929 
8930 
8931 
8932 
8933 
8934 
8935 
8936 
8937 
8938 
8939 
8940 
8941 
8942 
8943 
8944 
8945 
8946 
8947 
8948 
8949 
8950 // Parsing
8951 
8952 char whitespace[] = " \t\r\n\v";
8953 char symbols[] = "<|>&;()";
8954 
8955 int
8956 gettoken(char **ps, char *es, char **q, char **eq)
8957 {
8958   char *s;
8959   int ret;
8960 
8961   s = *ps;
8962   while(s < es && strchr(whitespace, *s))
8963     s++;
8964   if(q)
8965     *q = s;
8966   ret = *s;
8967   switch(*s){
8968   case 0:
8969     break;
8970   case '|':
8971   case '(':
8972   case ')':
8973   case ';':
8974   case '&':
8975   case '<':
8976     s++;
8977     break;
8978   case '>':
8979     s++;
8980     if(*s == '>'){
8981       ret = '+';
8982       s++;
8983     }
8984     break;
8985   default:
8986     ret = 'a';
8987     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
8988       s++;
8989     break;
8990   }
8991   if(eq)
8992     *eq = s;
8993 
8994   while(s < es && strchr(whitespace, *s))
8995     s++;
8996   *ps = s;
8997   return ret;
8998 }
8999 
9000 int
9001 peek(char **ps, char *es, char *toks)
9002 {
9003   char *s;
9004 
9005   s = *ps;
9006   while(s < es && strchr(whitespace, *s))
9007     s++;
9008   *ps = s;
9009   return *s && strchr(toks, *s);
9010 }
9011 
9012 struct cmd *parseline(char**, char*);
9013 struct cmd *parsepipe(char**, char*);
9014 struct cmd *parseexec(char**, char*);
9015 struct cmd *nulterminate(struct cmd*);
9016 
9017 struct cmd*
9018 parsecmd(char *s)
9019 {
9020   char *es;
9021   struct cmd *cmd;
9022 
9023   es = s + strlen(s);
9024   cmd = parseline(&s, es);
9025   peek(&s, es, "");
9026   if(s != es){
9027     printf(2, "leftovers: %s\n", s);
9028     panic("syntax");
9029   }
9030   nulterminate(cmd);
9031   return cmd;
9032 }
9033 
9034 struct cmd*
9035 parseline(char **ps, char *es)
9036 {
9037   struct cmd *cmd;
9038 
9039   cmd = parsepipe(ps, es);
9040   while(peek(ps, es, "&")){
9041     gettoken(ps, es, 0, 0);
9042     cmd = backcmd(cmd);
9043   }
9044   if(peek(ps, es, ";")){
9045     gettoken(ps, es, 0, 0);
9046     cmd = listcmd(cmd, parseline(ps, es));
9047   }
9048   return cmd;
9049 }
9050 struct cmd*
9051 parsepipe(char **ps, char *es)
9052 {
9053   struct cmd *cmd;
9054 
9055   cmd = parseexec(ps, es);
9056   if(peek(ps, es, "|")){
9057     gettoken(ps, es, 0, 0);
9058     cmd = pipecmd(cmd, parsepipe(ps, es));
9059   }
9060   return cmd;
9061 }
9062 
9063 struct cmd*
9064 parseredirs(struct cmd *cmd, char **ps, char *es)
9065 {
9066   int tok;
9067   char *q, *eq;
9068 
9069   while(peek(ps, es, "<>")){
9070     tok = gettoken(ps, es, 0, 0);
9071     if(gettoken(ps, es, &q, &eq) != 'a')
9072       panic("missing file for redirection");
9073     switch(tok){
9074     case '<':
9075       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
9076       break;
9077     case '>':
9078       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9079       break;
9080     case '+':  // >>
9081       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9082       break;
9083     }
9084   }
9085   return cmd;
9086 }
9087 
9088 
9089 
9090 
9091 
9092 
9093 
9094 
9095 
9096 
9097 
9098 
9099 
9100 struct cmd*
9101 parseblock(char **ps, char *es)
9102 {
9103   struct cmd *cmd;
9104 
9105   if(!peek(ps, es, "("))
9106     panic("parseblock");
9107   gettoken(ps, es, 0, 0);
9108   cmd = parseline(ps, es);
9109   if(!peek(ps, es, ")"))
9110     panic("syntax - missing )");
9111   gettoken(ps, es, 0, 0);
9112   cmd = parseredirs(cmd, ps, es);
9113   return cmd;
9114 }
9115 
9116 struct cmd*
9117 parseexec(char **ps, char *es)
9118 {
9119   char *q, *eq;
9120   int tok, argc;
9121   struct execcmd *cmd;
9122   struct cmd *ret;
9123 
9124   if(peek(ps, es, "("))
9125     return parseblock(ps, es);
9126 
9127   ret = execcmd();
9128   cmd = (struct execcmd*)ret;
9129 
9130   argc = 0;
9131   ret = parseredirs(ret, ps, es);
9132   while(!peek(ps, es, "|)&;")){
9133     if((tok=gettoken(ps, es, &q, &eq)) == 0)
9134       break;
9135     if(tok != 'a')
9136       panic("syntax");
9137     cmd->argv[argc] = q;
9138     cmd->eargv[argc] = eq;
9139     argc++;
9140     if(argc >= MAXARGS)
9141       panic("too many args");
9142     ret = parseredirs(ret, ps, es);
9143   }
9144   cmd->argv[argc] = 0;
9145   cmd->eargv[argc] = 0;
9146   return ret;
9147 }
9148 
9149 
9150 // NUL-terminate all the counted strings.
9151 struct cmd*
9152 nulterminate(struct cmd *cmd)
9153 {
9154   int i;
9155   struct backcmd *bcmd;
9156   struct execcmd *ecmd;
9157   struct listcmd *lcmd;
9158   struct pipecmd *pcmd;
9159   struct redircmd *rcmd;
9160 
9161   if(cmd == 0)
9162     return 0;
9163 
9164   switch(cmd->type){
9165   case EXEC:
9166     ecmd = (struct execcmd*)cmd;
9167     for(i=0; ecmd->argv[i]; i++)
9168       *ecmd->eargv[i] = 0;
9169     break;
9170 
9171   case REDIR:
9172     rcmd = (struct redircmd*)cmd;
9173     nulterminate(rcmd->cmd);
9174     *rcmd->efile = 0;
9175     break;
9176 
9177   case PIPE:
9178     pcmd = (struct pipecmd*)cmd;
9179     nulterminate(pcmd->left);
9180     nulterminate(pcmd->right);
9181     break;
9182 
9183   case LIST:
9184     lcmd = (struct listcmd*)cmd;
9185     nulterminate(lcmd->left);
9186     nulterminate(lcmd->right);
9187     break;
9188 
9189   case BACK:
9190     bcmd = (struct backcmd*)cmd;
9191     nulterminate(bcmd->cmd);
9192     break;
9193   }
9194   return cmd;
9195 }
9196 
9197 
9198 
9199 
