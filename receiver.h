#ifndef RECEIVER_H
#define RECEIVER_H

#include "geral.h"

/*
*Function: Opens the channel to communicate as receiver
*Return: returns 1 on sucess, -1 on error
*/
int llopen_receiver(linkLayer connectionParameters, int fd);
/*
*Function: Closes the communication as receiver
*Return: returns 1 on sucess, -1 on error
*/
int llclose_receiver(linkLayer connectionParameters); // NAO SEI QUAIS SERAO OS PARAMETROS NECESSARIOS

#endif