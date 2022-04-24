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
#define C_random1 0x05
#define C_S0 0x00
#define C_S1 0x02
#define C_RR0 0x01
#define C_RR1 0x21
#define C_REJ0 0x05
#define C_REJ1 0x25

#define ESC 0x7D


#define XOR 0x20

#define STATE0 0 // Start
#define STATE1 1 // Flag RCV
#define STATE2 2 // A RCV
#define STATE3 3 // C RCV
#define STATE4 4 // BCC OK
#define STATE5 5 // STOP
// para maquinas mais avançadas
#define STATE6 6
#define STATE7 7
#define STATE8 8


typedef struct stats{
    int num_frames;
    int num_bytes;
    int num_databytes;
    int num_timouts;
    int num_REJ;
} stats;

#endif