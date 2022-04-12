#ifndef TRANSMITER_H
#define TRANSMITER_H

#include "geral.h"

/*
*Function: Opens the channel to communicate as transmiter
*Return: returns 1 on sucess, -1 on error
*/
int llopen_transmiter(linkLayer connectionParameters, int fd);
/*
*Function: Closes the communication as transmitter
*Return: returns 1 on sucess, -1 on error
*/
int llclose_transmiter(linkLayer connectionParameters); // NAO SEI QUAIS SERAO OS PARAMETROS NECESSARIOS
/*
*
*Returno: none
*/
void timeout();

#endif