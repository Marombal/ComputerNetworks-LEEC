#ifndef GERAL_H
#define GERAL_H

#include "linklayer.h"
#include "defines.h"

static struct termios oldtio, newtio;


int setupSerialTerminal(linkLayer connectionParameters);
int closeSerialTerminal(int fd);
int setupFrameFormat(char* buf, char* frame, int bufSize);
int resetFrameFormat(char* buf, int bufSize);
int stuffing(char* buf, int bufSize);
int destuffing(char* buf, int bufSize);
speed_t convertBaudRate(int baud);
void imprime(char* buf, int bufSize);
char calculaBCC(char* frame, int frameSize);
void Statistics(stats tats_);
int compara(char* vetor1, char* vetor2, int size);
int verificaBCC(char* frame, int frameSize);
void resetStats(stats stats_);
char toggleNs(char Ns);
char toggleRR(char Nr);
char toggleREJ(char Nr);

#endif