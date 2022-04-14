/* INCLUDES .h*/
#include "linklayer.h"
#include "receiver.h"
#include "transmiter.h"
#include "geral.h"
#include "defines.h"

static struct termios oldtio, newtio;   
static int fd;

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters)
{
    
    fd = setupSerialTerminal(connectionParameters);
    if(connectionParameters.role == RECEIVER){
        //... abre como receiver
        return llopen_receiver(connectionParameters, fd); 
    }
    if(connectionParameters.role == TRANSMITTER){
        //... abre como transmiter
        return llopen_transmiter(connectionParameters, fd); 
    }

    return -1;
}
// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize){
    // controlo dos parametros
    if((!buf)||(bufSize<0)||(bufSize>MAX_PAYLOAD_SIZE)) return -1;
        
    char frame[MAX_PAYLOAD_SIZE*2];  /*frame auxiliar*/

    /* Controlo */
    printf("llwrite here... im will setup this: %s\n", buf);
    imprime(buf, bufSize);

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
    
    sleep(1);
    /* Write */
    int res = write(fd, frame, new_bufSize); 
    if(res == -1)   return -1;

    return new_bufSize;
}
// Receive data in packet
int llread(char* packet){
    // controlo dos parametros
    if(!packet) return -1;

    /* Controlo */
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

    return packetSize;    
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics){
    /*if(connectionParameters.role == RECEIVER){
        //... abre como receiver
        return llclose_receiver(connectionParameters); 
    }
    if(connectionParameters.role == TRANSMITTER){
        //... abre como transmiter
        return llclose_transmiter(connectionParameters); */
    return -1;
}