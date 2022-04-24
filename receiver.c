#include "receiver.h"

/*
*Function: Opens the channel to communicate as receiver
*Return: returns 1 on sucess, -1 on error
*/
int llopen_receiver(linkLayer connectionParameters, int fd){
    // NONCANONICAL

    unsigned char UA[] = {FLAG, A_2, C, BCC_2, FLAG}; // Definição do UA segundo o protocolo    
    int res;
    int STOP = FALSE;
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
    //sleep(4); // <--- tests
    res = write(fd, UA, 5);
    printf("UA (enviado): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", UA[0], UA[1], UA[2], UA[3], UA[4]);


    return 1;
}
