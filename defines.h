#ifndef DEFINES_H
#define DEFINES_H


#define FLAG 0x7e
#define A_1 0x03 // A_tx
#define A_2 0x01 // A_rx

#define BCC_1 (A_1^C)
#define BCC_2 (A_2^C)

#define BCC_DISC_tx (A_1^C_DISC)
#define BCC_DISC_rx (A_2^C_DISC)

#define C 0x03
#define C_SET 0x03
#define C_DISC 0x0b
#define C_UA 0x07

#define C_random 0x04
//#define C_RR 0x00
//#define C_REJ 0x00

#define ESC 0x7D


#define XOR 0x20

#define STATE0 0 // Start
#define STATE1 1 // Flag RCV
#define STATE2 2 // A RCV
#define STATE3 3 // C RCV
#define STATE4 4 // BCC OK
#define STATE5 5 // STOP


typedef struct stats{
    char serialPort[100][1000];
    int num_retransmitted_frames;
    int num_received_frames;
    int num_timouts;
    int num_REJ;
} stats;

#endif