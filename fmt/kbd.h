7800 // PC keyboard interface constants
7801 
7802 #define KBSTATP         0x64    // kbd controller status port(I)
7803 #define KBS_DIB         0x01    // kbd data in buffer
7804 #define KBDATAP         0x60    // kbd data port(I)
7805 
7806 #define NO              0
7807 
7808 #define SHIFT           (1<<0)
7809 #define CTL             (1<<1)
7810 #define ALT             (1<<2)
7811 
7812 #define CAPSLOCK        (1<<3)
7813 #define NUMLOCK         (1<<4)
7814 #define SCROLLLOCK      (1<<5)
7815 
7816 #define E0ESC           (1<<6)
7817 
7818 // Special keycodes
7819 #define KEY_HOME        0xE0
7820 #define KEY_END         0xE1
7821 #define KEY_UP          0xE2
7822 #define KEY_DN          0xE3
7823 #define KEY_LF          0xE4
7824 #define KEY_RT          0xE5
7825 #define KEY_PGUP        0xE6
7826 #define KEY_PGDN        0xE7
7827 #define KEY_INS         0xE8
7828 #define KEY_DEL         0xE9
7829 
7830 // C('A') == Control-A
7831 #define C(x) (x - '@')
7832 
7833 static uchar shiftcode[256] =
7834 {
7835   [0x1D] CTL,
7836   [0x2A] SHIFT,
7837   [0x36] SHIFT,
7838   [0x38] ALT,
7839   [0x9D] CTL,
7840   [0xB8] ALT
7841 };
7842 
7843 static uchar togglecode[256] =
7844 {
7845   [0x3A] CAPSLOCK,
7846   [0x45] NUMLOCK,
7847   [0x46] SCROLLLOCK
7848 };
7849 
7850 static uchar normalmap[256] =
7851 {
7852   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
7853   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
7854   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
7855   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
7856   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
7857   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
7858   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
7859   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7860   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7861   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7862   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7863   [0x9C] '\n',      // KP_Enter
7864   [0xB5] '/',       // KP_Div
7865   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7866   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7867   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7868   [0x97] KEY_HOME,  [0xCF] KEY_END,
7869   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7870 };
7871 
7872 static uchar shiftmap[256] =
7873 {
7874   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
7875   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
7876   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
7877   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
7878   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
7879   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
7880   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
7881   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7882   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7883   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7884   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7885   [0x9C] '\n',      // KP_Enter
7886   [0xB5] '/',       // KP_Div
7887   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7888   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7889   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7890   [0x97] KEY_HOME,  [0xCF] KEY_END,
7891   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7892 };
7893 
7894 
7895 
7896 
7897 
7898 
7899 
7900 static uchar ctlmap[256] =
7901 {
7902   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7903   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7904   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
7905   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
7906   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
7907   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
7908   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
7909   [0x9C] '\r',      // KP_Enter
7910   [0xB5] C('/'),    // KP_Div
7911   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7912   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7913   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7914   [0x97] KEY_HOME,  [0xCF] KEY_END,
7915   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7916 };
7917 
7918 
7919 
7920 
7921 
7922 
7923 
7924 
7925 
7926 
7927 
7928 
7929 
7930 
7931 
7932 
7933 
7934 
7935 
7936 
7937 
7938 
7939 
7940 
7941 
7942 
7943 
7944 
7945 
7946 
7947 
7948 
7949 
