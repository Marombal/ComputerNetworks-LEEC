#include "transmiter.h"


//volatile int STOP=FALSE;

int n_timeouts=0, flag=1;
void timeout()                   // atende alarme
{
	printf("timeout #%d...\n", (n_timeouts+1));
	flag=1;
	n_timeouts++;
    
}
/*
*Function: Opens the channel to communicate as transmiter
*Return: returns 1 on sucess, -1 on error
*/
int llopen_transmiter(linkLayer connectionParameters, int fd){
    // WRITE NONCANONICAL
    n_timeouts=0, flag=1;
    int res;

    unsigned char SET[] = {FLAG, A_1, C, BCC_1, FLAG}; // Definição do SET segundo o protocolo
    
    int STOP=FALSE;
    /* Envio do set atravez da função write */
    res = write(fd,SET,5); 
    //printf("%d bytes written\n", res);
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
      if((n_timeouts<connectionParameters.numTries)&&(STATE!=STATE5)){          
        if(flag){
          alarm(connectionParameters.timeOut);
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

    if(n_timeouts>=connectionParameters.numTries){
      printf("Failed to start conection...\n");
      return -1;
    }
    if(STATE==STATE5){
      printf("UA (recebido): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", AUX[0], AUX[1], AUX[2], AUX[3], AUX[4]);
      return 1;
    }
    
    // ERRO
    return -1;
}
/*
*Function: Closes the communication as transmiter
*Return: returns 1 on sucess, -1 on error
*/
int llclose_transmiter(linkLayer connectionParameters){
    return -1;
}
// NAO SEI QUAIS SERAO OS PARAMETROS NECESSARIOS