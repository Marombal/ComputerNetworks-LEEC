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
    newtio.c_cflag = convertBaudRate(connectionParameters.baudRate) | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = (connectionParameters.timeOut * 10);   /* inter-character timer unused */
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
  sleep(1);
  close(fd);
  return 1;
}
      

/*
*Function:
*Return:
*/
int setupFrameFormat(char* buf, char* frame, int bufSize){
  if((!buf)||(bufSize<0)) return -1;
  char BCC2 = calculaBCC(buf, bufSize);

      frame[0] = FLAG;
      frame[1] = A_1;
      frame[2] = C;
      frame[3] = BCC_1; // MALMALMALASDJIAHSDUIOAHDUHWADIUHWUIDHAUDHUAIH
      for(int i = 0; i < bufSize; i++){
          frame[i+4] = buf[i];
      }
      
      frame[bufSize + 4] = BCC2;
      frame[bufSize + 5] = FLAG;

      //imprime(frame, size+6);
      return bufSize+6; /// MUITO CUIDADO
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

    for(int i = 3; i < bufSize-1; i++){
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

    for(int i = 3; i<bufSize-1; i++){
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


speed_t convertBaudRate(int baud){
    switch (baud) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    default: 
        return -1;
    }
}

char calculaBCC(char* frame, int frameSize){
  char BCC2 = 0x00;
  for(int k = 0; k < frameSize; k++){
    BCC2 ^= frame[k];
    
  }
  return BCC2;
}

void Statistics(stats stats_){
  printf("\n\n");
  printf("/********************************************************************/\n");
  printf("/*****************************Statistics*****************************/\n");
  printf("/********************************************************************/\n");
  printf("\n");
  //STATS
  printf("Number of received/trasmitted frames: %d\n", stats_.num_frames);
  printf("Number of received/trasmitted BYTES -> Total: %d (%d of data/%d of overhead)\n", stats_.num_bytes, stats_.num_databytes, stats_.num_bytes-stats_.num_databytes);
  printf("Number of REJ: %d\n", stats_.num_REJ);
  printf("Number of timeouts: %d\n", stats_.num_timouts);
  //
  printf("\n\n");
  return;
}


void resetStats(stats stats_){
  stats_.num_REJ = 0;
  stats_.num_timouts = 0;
  stats_.num_frames = 0;
  stats_.num_bytes = 0;
  stats_.num_databytes = 0;
}

char toggleNs(char Ns){
  if(Ns == C_S0) return C_S1;
  else return C_S0;
}

/*
* Funcao: Imprimir no formato hexadecimal o conteudo do parametro "buf"
*/
void imprime(char* buf, int bufSize){
  if((!buf)||(bufSize<0)) return;
  for(int i = 0; i < bufSize; i++){
    printf("%02x ", buf[i]);
  }
  printf("\n");
}