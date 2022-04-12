#ifndef DEFINES_H
#define DEFINES_H


#define FLAG 0x7e
#define A_1 0x03 // A_tx
#define A_2 0x01 // A_rx
#define C 0x03
#define BCC_1 A_1^C
#define BCC_2 A_2^C


#define ESC 0x7D


#define STATE0 0 // Start
#define STATE1 1 // Flag RCV
#define STATE2 2 // A RCV
#define STATE3 3 // C RCV
#define STATE4 4 // BCC OK
#define STATE5 5 // STOP

#endif