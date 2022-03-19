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
*/


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS5", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


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
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



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

    char string[255];
    int aux = 0, res2;
    while (STOP==FALSE) {       /* loop for input */
      
      res = read(fd,buf,1);   //puts(buf);
      
      string[aux] = buf[0];
      aux++;
      
      if (buf[0]=='\0') STOP=TRUE;
      
    }
    //printf("str:%s tam:%d\n", buf, res);
    puts(string);
    
    string[0] = ('!'); // para testar 
    
    res = write(fd, string, (strlen(string)+1));
    
    //res2 = select(1, 2, NULL, NULL, &5)  
    //multiple_timerc()
    


  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    sleep(1);
    close(fd);
    return 0;
}
