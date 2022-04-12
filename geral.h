#ifndef GERAL_H
#define GERAL_H

#include "linklayer.h"
#include "defines.h"

static struct termios oldtio, newtio;

int setupSerialTerminal(linkLayer connectionParameters);
int closeSerialTerminal(int fd);
int setupFrameFormat(char* buf, int bufSize);
int resetFrameFormat(char* buf);
int stuffing(char* buf, int bufSize);
int destuffing(char* buf, int bufSize);

#endif