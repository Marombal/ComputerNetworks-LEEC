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
int baudRateConvert(int baud);
void imprime(char* buf, int bufSize);

#endif