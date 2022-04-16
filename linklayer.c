/* INCLUDES .h*/
#include "linklayer.h"
#include "receiver.h"
#include "transmiter.h"
#include "geral.h"
#include "defines.h"

static struct termios oldtio, newtio;   
static int fd;
static int numTries_Receiver, timeOut_Receiver;
static int numTries_Transmiter, timeOut_Transmiter;
static int TYPE; // TYPE = 0 (TX) | TYPE = 1 (RX)
int timeouts = 0, flag_ = 1;

struct stats stats_;


void timeout_()                   // atende alarme
{
	printf("timeout: #%d...\n", (timeouts+1));
	flag_=1;
	timeouts++;
}

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters){
    
    fd = setupSerialTerminal(connectionParameters);
    if(connectionParameters.role == RECEIVER){
        //... abre como receiver
        TYPE = connectionParameters.role;
        numTries_Receiver = connectionParameters.numTries;
        timeOut_Receiver = connectionParameters.numTries;
        return llopen_receiver(connectionParameters, fd); 
    }
    if(connectionParameters.role == TRANSMITTER){
        //... abre como transmiter
        TYPE = connectionParameters.role;
        numTries_Transmiter = connectionParameters.numTries;
        timeOut_Transmiter = connectionParameters.numTries;
        return llopen_transmiter(connectionParameters, fd); 
    }

    return -1;
}

/* Sends data in buf with size bufSize
* Function: 
* Return: "-1" on error and "1" on sucess
*/
int llwrite(char* buf, int bufSize){
    // controlo dos parametros
    if((!buf)||(bufSize<0)||(bufSize>MAX_PAYLOAD_SIZE)) return -1;
        
    char frame[MAX_PAYLOAD_SIZE*2];  /*frame auxiliar*/

    /* Controlo */
    printf("llwrite here... im will setup this: %s\n", buf);
    imprime(buf, bufSize);

    /*PARTE 1 - PREPARAR O FRAME************************************************/
    /* FrameFormat */
    int new_bufSize = setupFrameFormat(buf, frame, bufSize);
    if(new_bufSize == -1)   return -1;
    printf("after setting frame format the frame looks like this: \n");
    imprime(frame, new_bufSize);

    /* Stuffing */
    new_bufSize = stuffing(frame, new_bufSize);
    if(new_bufSize == -1)   return -1;
    printf("after stuffing the frame looks like this: \n");
    imprime(frame, new_bufSize);
    /****************************************************************************/
    /*PARTE 2 - ENVIAR O FRAME***************************************************/
    printf("Sending...              ");
    sleep(1);
    /* Write */
    int res = write(fd, frame, new_bufSize); 
    if(res == -1)   return -1;
    else printf("DONE :)\n");
    /****************************************************************************/
    /*PARTE 3 - LER A RESPOSTA DO RECEIVER E AGIR EM FUNÇÃO DISSO****************/
    // Se em (timeOut_Transmiter) segundos nao receber nada reenviar 
    // Se receber REJ reenviar
        // O reenvio so deve ser feito um total de numTries_Transmiter vezes, após isso return ERRO (-1)
    // Se receber ACK correto retornar SUCEESSO (numero de bytes enviados (>0))

    /* Listen to Receiver's answer */
    printf("Now im waiting ur answer \n");
    (void) signal(SIGALRM, timeout_);
    int STOP = FALSE, count = 0, i = 0;
    char answer[5], AUX_1;


    while (STOP==FALSE) /* loop for input */ 
    {   
        if(timeouts < numTries_Transmiter){
            if(flag_){
                alarm(timeOut_Transmiter);
                flag_ = 0;
                if(timeouts!=0){           // after 1st timeout
                    printf("Sending again, wish u luck\n");
                    res = write(fd, frame, new_bufSize);    // send all the frame again   
                }
            }
        }
        else{
            alarm(0);
            STOP = TRUE;
        }
        
        res = read(fd, &AUX_1, 1);
        //printf("recebi: %02x\n", AUX_1);
        answer[i++] = AUX_1;
        if(AUX_1 == FLAG)   count++;
        if(count == 2)  STOP = TRUE;
    }
    alarm(0);
    int error = 0;
    char REJ[] = {FLAG, A_1, C_random, BCC_1, FLAG};
    char RR[] = {FLAG, A_1, C_random, BCC_1, FLAG}; //MAL OS COISOS
    imprime(answer, i);
    for(int h = 0; h < 5; h++){
        if(answer[h]!=RR[h]) error = 1;
    }
    if(error == 1){
        printf("Ohhh no, something went wrong here :( \n");
    }
    else{
        printf("THATS AN ACK! PERFERCT :) \n");
    }

    return new_bufSize;
}
// Receive data in packet
int llread(char* packet){
    // controlo dos parametros
    if(!packet) return -1;

    /* Leitura */
    int res, STOP = FALSE, STATE = 0, count=0, packetSize = 0;
    unsigned char AUX[255], AUX_1 = 'd';
    printf("llread here... lets see what i receive\n");
    while (STOP==FALSE) /* loop for input */ 
    {   
        res = read(fd, &AUX_1, 1);
        packet[packetSize++] = AUX_1;
        if(AUX_1 == FLAG)   count++;
        if(count == 2)  STOP = TRUE;
    }
    imprime(packet, packetSize);

    packetSize = resetFrameFormat(packet, packetSize);
    if(packetSize == -1)    return -1;
    printf("after reseting frame format the frame looks like this: \n");
    imprime(packet, packetSize);

    packetSize = destuffing(packet, packetSize);
    if(packetSize == -1)    return -1; 
    printf("after destuffing the frame looks like this: \n");
    imprime(packet, packetSize);

    /* Verifica se está tudo bem */
    int ready = 1; //ready = -1 quando ocorreu um erro e é suposto enviar um reject; = 1 quando está tudo bem
    // por fazer

    sleep(15); //maybe not necessario
    /* Envia a resposta */
    if(ready == -1){
        //REJ
        printf("Something went wrong here :( Sending REJ...\n");
        char REJ[] = {FLAG, A_1, C_random, BCC_1, FLAG};    //MAL OS COISOS
        res = write(fd, REJ, 5);
    }
    else{
        //RR
        printf("Everything seems cool :) Sending ACK...\n");
        char RR[] = {FLAG, A_1, C_random, BCC_1, FLAG};    //MAL OS COISOS
        res = write(fd, RR, 5);
        imprime(RR, 5);
    }
   
    return packetSize;    
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics){

    if(TYPE == RECEIVER){
        //... abre como receiver
        //return llclose_receiver(); 
        if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
        }
        sleep(1);
        close(fd);
        if(showStatistics)  Statistics();
        return 1;
    }
    if(TYPE == TRANSMITTER){
        //... abre como transmiter
        //return llclose_transmiter();
        if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
        }
        sleep(1);
        close(fd);
        return 1;

    }
    return -1;
}