/* INCLUDES .h*/
#include "linklayer.h"
#include "receiver.h"
#include "transmiter.h"
#include "geral.h"
#include "defines.h"

static struct termios oldtio, newtio;   
int fd;

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters)
{
    
    int fd = setupSerialTerminal(connectionParameters);

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
    if((!buf)||(bufSize<0)||(bufSize>MAX_PAYLOAD_SIZE))
        return -1;
    
    int new_bufSize = setupFrameFormat(buf, bufSize);
    if(new_bufSize == -1)   return -1;
    new_bufSize = stuffing(buf, new_bufSize);
    if(new_bufSize == -1)   return -1;
    int res = write(fd,buf,new_bufSize); 
    if(res == -1)   return -1;

    return new_bufSize;
}
// Receive data in packet
int llread(char* packet){
    // controlo dos parametros
    if(!packet)
        return -1;
    int packetSize = resetFrameFormat(packet);
    if(packetSize == -1)    return -1;
    packetSize = destuffing(packet, packetSize);
    if(packetSize == -1)    return -1; 

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