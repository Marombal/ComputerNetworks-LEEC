#include "geral.h"


/*
*Function: Setup the comunnication
*Return: returns fd (file descriptor) on sucess, -1 on error
*/
int setupSerialTerminal(linkLayer connectionParameters){
    int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(connectionParameters.serialPort); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    return fd;
}

/*
*Function: Closes the communication
*Return: returns 1 on sucess, -1 on error
*/
int closeSerialTerminal(int fd){
  //tcflush(fd, TCIOFLUSH);
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  close(fd);
  return 1;
}

/*
*Function:
*Return:
*/
int setupFrameFormat(char* buf, int bufSize){
  return -1;
}

/*
*Function:
*Return:
*/
int resetFrameFormat(char* buf){
  return -1;
}


/*
*Function: Stuffs the buf
*Return: The size of the new buf on sucess, -1 on error
*/
int stuffing(char* buf, int bufSize){
    return -1;
}

/*
*Function: Destuffs the buf
*Return: The size of the new buf on sucess, -1 on error
*/
int destuffing(char* buf, int bufSize){
    return -1;
}