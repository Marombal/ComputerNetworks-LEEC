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

    newtio.c_cc[VTIME]    = 30;   /* inter-character timer unused */
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
int setupFrameFormat(char* buf, char* frame, int bufSize){
  if((!buf)||(bufSize<0)) return -1;
      
      frame[0] = FLAG;
      frame[1] = A_1;
      frame[2] = C;
      frame[3] = BCC_1; // MALMALMALASDJIAHSDUIOAHDUHWADIUHWUIDHAUDHUAIH
      for(int i = 0; i < bufSize; i++){
          frame[i+4] = buf[i];
      }
      frame[bufSize + 4] = BCC_2;
      frame[bufSize + 5] = FLAG;

      //imprime(frame, size+6);
      return bufSize+6;
}

/*
*Function:
*Return:
*/
int resetFrameFormat(char* buf, int bufSize){
  if((!buf)||(bufSize<0)) return -1;
      /* Tirar a FLAG, A, C, BCC do inicio do buf */
      for(int i = 0; i < bufSize - 2; i++){
          buf[i] = buf[i+4];
      }
      return bufSize - 6;
}


/*
*Function: Stuffs the buf
*Return: The size of the new buf on sucess, -1 on error
*/
int stuffing(char* buf, int bufSize){
  if((!buf)||(bufSize<0)) return -1;
    
    char aux1, aux2;

    for(int i = 3; i < bufSize-2; i++){
        if(buf[i] == ESC){
            bufSize++;
            aux1 = buf[i+1];
            buf[i+1] = (ESC^XOR);    //this should be 0x5e
            for(int j = i+2; j <= bufSize ;j++){
                aux2 = buf[j];
                buf[j]=aux1;
                aux1 = aux2;
            }
        }
    }

    for(int i = 3; i<bufSize-2; i++){
        if(buf[i] == FLAG){
            buf[i]=ESC;
            bufSize++;
            aux1=buf[i+1];
            buf[i+1]=(FLAG^XOR);  //this should be 
            for(int j = i+2; j<=bufSize; j++){
                aux2 = buf[j];
                buf[j]=aux1;
                aux1=aux2;
            }
        }
    }
    return bufSize;
}

/*
*Function: Destuffs the buf
*Return: The size of the new buf on sucess, -1 on error
*/
int destuffing(char* buf, int bufSize){
  if((!buf)||(bufSize<0)) return -1;

  for(int i = 0; i < bufSize; i++){
    if(buf[i] == ESC){
      if(buf[i+1] == (FLAG^XOR)){
        for(int j = i; j < bufSize; j++){                
          buf[j] = buf[j+1];
        }
          bufSize--;
          buf[i] = FLAG;
      }
      else if(buf[i+1] == (ESC^XOR)){
        for(int j = i; j < bufSize; j++){
          buf[j] = buf[j+1];
        }
        bufSize--;
        buf[i] = ESC;
      }
      else{
        return -1;}
    }
  }
  return bufSize;
}

int baudRateConvert(int baud){
  switch (baud)
  {
  case 9600:
    return B9600;
    break;
  }
  return -1;
}

void imprime(char* buf, int bufSize){
  if((!buf)||(bufSize<0)) return;
  for(int i = 0; i < bufSize; i++){
    printf("%02x ", buf[i]);
  }
  printf("\n");
}