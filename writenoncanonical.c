
/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
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

int n_timeouts=0, flag=1;
void timeout()                   // atende alarme
{
	printf("timeout #%d...\n", (n_timeouts+1));
	flag=1;
	n_timeouts++;
}


/*
while(conta < 4){
   if(flag){
      alarm(3);                 // activa alarme de 3s
      flag=0;
   }
*/

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
/*   
    if ( (argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1])!=0) &&
       (strcmp("/dev/ttyS1", argv[1])!=0) )) {
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
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */



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

    unsigned char SET[] = {FLAG, A_1, C, BCC_1, FLAG}; // Definição do SET segundo o protocolo
    
    /* Envio do set atravez da função write */
    res = write(fd,SET,5); 
    printf("%d bytes written\n", res);
    printf("SET (enviado): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", SET[0], SET[1], SET[2], SET[3], SET[4]);
 
    /*
      ### Variaveis auxiliares ao while ###

      aux - para determinar o indice do vetor 
      para - para determinar o fim da mensagem (na segunda flag)
      AUX - vetor onde se guardara a mensagem
      AUX_1 - onde se vai guardar a leitura que é feita 1 de cada vez
    */

    int aux = 0, para = 0;
    unsigned char AUX[255], AUX_1;
    int STATE = 0, contador = 0;
    
    (void) signal(SIGALRM, timeout);  // instala  rotina que atende interrupcao
    
    while (STOP==FALSE) /* loop for input */
    {       
      if((n_timeouts<3)&&(STATE!=STATE5)){
        if(flag){
          alarm(3);
          flag = 0;
          if(n_timeouts!=0){           // after 1st timeout
            res = write(fd,SET,5);    // send SET again
            //printf("Reenvio #%d\n", n_timeouts); 
          }
        }
      }
      else{
        STOP=TRUE;
        alarm(0);
      }
      
      res = read(fd,&AUX_1,1);
      
      AUX[STATE] = AUX_1;
      
      switch(STATE)
      {
        case (STATE0):
            if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE1):
            if(AUX_1 == A_2) STATE = STATE2;
            else if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE2):
            if(AUX_1 == C) STATE = STATE3;
            else if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE3):
            if(AUX_1 == BCC_2) STATE = STATE4;
            else if(AUX_1 == FLAG) STATE = STATE1;
            else STATE = STATE0;
            break;
        case (STATE4):
            if(AUX_1 == FLAG) STATE = STATE5;
            else STATE = STATE0;
            break;
      }  
      
      if (STATE == STATE5)
      { 
        STOP = TRUE;
        para = 1;
        contador = 0;
      }
    }

    if(n_timeouts>=3){
      printf("Failed to start conection...\n");
    }
    if(STATE==STATE5){
      printf("UA (recebido): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", AUX[0], AUX[1], AUX[2], AUX[3], AUX[4]);
    }
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}

