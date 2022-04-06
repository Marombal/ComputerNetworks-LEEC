/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

//#include <sys/time.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A_1 0x03
#define A_2 0x01
#define C 0x03
#define BCC_1 A_1^C
#define BCC_2 A_2^C



#define STATE0 0 // Start
#define STATE1 1 // Flag RCV
#define STATE2 2 // A RCV
#define STATE3 3 // C RCV
#define STATE4 4 // BCC OK
#define STATE5 5 // STOP


/*  
    FLAG)
        All frames are delimited by flags (01111110)
    A)
        00000011 (0x03) in Commands sent by the Transmitter and Answers sent by the Receiver
        00000001 (0x01) in Commands sent by the Receiver and Answers sent by the Transmitter
    C)
        C (Control fiels) ¨C Defines the type of frame and carries the sequence numbers
        N(s) in I frames and N(r) in S frames (RR, REJ)
    BCC)
        BCC (Block Check Character) ¨C Provides error control based on an octet that
        guarantees that there is an even pair of 1¡¯s (even parity) for each bit position,
        considering all octets protected by the BCC (header or data) and the BCC
        (before stuffing)
    SET = [FLAG, A, C, BCC, FLAG]
    UA = [FLAG, A, C, BCC, FLAG]
*/


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
/*
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS5", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
*/

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */


    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
    
    unsigned char UA[] = {FLAG, A_2, C, BCC_2, FLAG}; // Definição do UA segundo o protocolo
    
    int STATE = 0;
    /*
      ### Variaveis auxiliares ao while ###

      aux - para determinar o indice do vetor 
      para - para determinar o fim da mensagem (na segunda flag)
      AUX - vetor onde se guardara a mensagem
      AUX_1 - onde se vai guardar a leitura que é feita 1 de cada vez
    */

    unsigned char AUX[255], AUX_1;
    int aux = 0, para = 0;

    while (STOP==FALSE) /* loop for input */ 
    {        
      res = read(fd,&AUX_1,1);
      AUX[STATE] = AUX_1;
      //printf("AUX: %02X STATE: %d \n", AUX[STATE], STATE);
      switch(STATE)
      {
        case (STATE0):
            if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE1):
            if(AUX_1 == A_1) STATE = STATE2;
            else if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE2):
            if(AUX_1 == C) STATE = STATE3;
            else if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE3):
            if(AUX_1 == BCC_1) STATE = STATE4;
            else if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE4):
            if(AUX_1 == FLAG) STATE = STATE5;
            else STATE = STATE0;
            break;
      }


      if (STATE == STATE5) STOP = TRUE;
    }
    printf("SET (Recebido): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", AUX[0], AUX[1], AUX[2], AUX[3], AUX[4]);

    /* Envio do UA de modo a confirmar a receção do SET */
    //sleep(5); // <--- tests
    res = write(fd, UA, 5);
    printf("UA (enviado): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", UA[0], UA[1], UA[2], UA[3], UA[4]);



    tcsetattr(fd,TCSANOW,&oldtio);
    sleep(1);
    close(fd);
    return 0;
}
