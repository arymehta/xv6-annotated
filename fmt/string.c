7000 #include "types.h"
7001 #include "x86.h"
7002 
7003 void*
7004 memset(void *dst, int c, uint n)
7005 {
7006   if ((int)dst%4 == 0 && n%4 == 0){
7007     c &= 0xFF;
7008     stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
7009   } else
7010     stosb(dst, c, n);
7011   return dst;
7012 }
7013 
7014 int
7015 memcmp(const void *v1, const void *v2, uint n)
7016 {
7017   const uchar *s1, *s2;
7018 
7019   s1 = v1;
7020   s2 = v2;
7021   while(n-- > 0){
7022     if(*s1 != *s2)
7023       return *s1 - *s2;
7024     s1++, s2++;
7025   }
7026 
7027   return 0;
7028 }
7029 
7030 void*
7031 memmove(void *dst, const void *src, uint n)
7032 {
7033   const char *s;
7034   char *d;
7035 
7036   s = src;
7037   d = dst;
7038   if(s < d && s + n > d){
7039     s += n;
7040     d += n;
7041     while(n-- > 0)
7042       *--d = *--s;
7043   } else
7044     while(n-- > 0)
7045       *d++ = *s++;
7046 
7047   return dst;
7048 }
7049 
7050 // memcpy exists to placate GCC.  Use memmove.
7051 void*
7052 memcpy(void *dst, const void *src, uint n)
7053 {
7054   return memmove(dst, src, n);
7055 }
7056 
7057 int
7058 strncmp(const char *p, const char *q, uint n)
7059 {
7060   while(n > 0 && *p && *p == *q)
7061     n--, p++, q++;
7062   if(n == 0)
7063     return 0;
7064   return (uchar)*p - (uchar)*q;
7065 }
7066 
7067 char*
7068 strncpy(char *s, const char *t, int n)
7069 {
7070   char *os;
7071 
7072   os = s;
7073   while(n-- > 0 && (*s++ = *t++) != 0)
7074     ;
7075   while(n-- > 0)
7076     *s++ = 0;
7077   return os;
7078 }
7079 
7080 // Like strncpy but guaranteed to NUL-terminate.
7081 char*
7082 safestrcpy(char *s, const char *t, int n)
7083 {
7084   char *os;
7085 
7086   os = s;
7087   if(n <= 0)
7088     return os;
7089   while(--n > 0 && (*s++ = *t++) != 0)
7090     ;
7091   *s = 0;
7092   return os;
7093 }
7094 
7095 
7096 
7097 
7098 
7099 
7100 int
7101 strlen(const char *s)
7102 {
7103   int n;
7104 
7105   for(n = 0; s[n]; n++)
7106     ;
7107   return n;
7108 }
7109 
7110 
7111 
7112 
7113 
7114 
7115 
7116 
7117 
7118 
7119 
7120 
7121 
7122 
7123 
7124 
7125 
7126 
7127 
7128 
7129 
7130 
7131 
7132 
7133 
7134 
7135 
7136 
7137 
7138 
7139 
7140 
7141 
7142 
7143 
7144 
7145 
7146 
7147 
7148 
7149 
