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
int timeouts = 0, flag_ = 1, conta = 0;
int func; // func = 0 on llopen; = 1 on llwrite/llread; = 2 on llclose

static struct stats stats_;

void timeout_()                   // atende alarme
{
	printf("timeout: #%d...\n", (timeouts+1));
	flag_ = 1;
	timeouts++;
    if(conta) stats_.num_timouts++;
}

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters){
    /* Reeniciar timeouts */
    timeouts = 0;
    conta = 0;  //  PARA AS ESTATISTICAS
    fd = setupSerialTerminal(connectionParameters);
    if(connectionParameters.role == RECEIVER){
        //... abre como receiver
        resetStats(stats_);
        TYPE = connectionParameters.role;
        numTries_Receiver = connectionParameters.numTries;
        timeOut_Receiver = connectionParameters.timeOut;
        return llopen_receiver(connectionParameters, fd); 
    }
    if(connectionParameters.role == TRANSMITTER){
        //... abre como transmiter
        resetStats(stats_);
        TYPE = connectionParameters.role;
        numTries_Transmiter = connectionParameters.numTries;
        timeOut_Transmiter = connectionParameters.timeOut;
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
        
    conta = 1;  //  PARA AS ESTATISTICAS
    
    /* Reeniciar timeouts */
    timeouts = 0;

    char frame[MAX_PAYLOAD_SIZE*2];  /*frame auxiliar*/
    int error = 0;
    int STOP = FALSE, count = 0, i = 0, flag_encontrada = 0;
    char answer[5], AUX_1;
    char REJ[] = {FLAG, A_1, C_random1, BCC_1, FLAG};
    char RR[] = {FLAG, A_1, C_random, BCC_1, FLAG}; //MAL OS COISOS

    /* Controlo */
    //printf("llwrite here... im will setup this: %s\n", buf);
    //imprime(buf, bufSize);

    /*PARTE 1 - PREPARAR O FRAME************************************************/
    /* FrameFormat */
    stats_.num_databytes += bufSize;

    int new_bufSize = setupFrameFormat(buf, frame, bufSize);
    if(new_bufSize == -1)   return -1;
    ///* Stuffing */

    new_bufSize = stuffing(frame, new_bufSize);
    if(new_bufSize == -1)   return -1;
    
    /*PARTE 2 - ENVIAR O FRAME***************************************************/
    //printf("Sending...              ");
    //sleep(1);
    int res = write(fd, frame, new_bufSize); 
    if(res == -1)   return -1;
    stats_.num_frames++;
    stats_.num_bytes += res;
    /****************************************************************************/
    /*PARTE 3 - LER A RESPOSTA DO RECEIVER E AGIR EM FUNÇÃO DISSO****************/
    // Se em (timeOut_Transmiter) segundos nao receber nada reenviar 
    // Se receber REJ reenviar
    // O reenvio so deve ser feito um total de numTries_Transmiter vezes, após isso return ERRO (-1)
    // Se receber ACK correto retornar SUCEESSO (numero de bytes enviados (>0))

    /* Listen to Receiver's answer */
    //printf("Now im waiting ur answer \n");
    (void) signal(SIGALRM, timeout_);

    while (STOP==FALSE) /* loop for input */ 
    {   
        if(timeouts < numTries_Transmiter){
            if(flag_){
                alarm(timeOut_Transmiter);
                flag_ = 0;
                if(timeouts!=0){           // after 1st timeout
                    printf("Sending again due to timeout. \n");
                    res = write(fd, frame, new_bufSize);    // send all the frame again   
                    if(res == -1) return -1;
                    stats_.num_frames++;
                    //stats_.num_bytes += new_bufSize;
                }
            }
        }
        else{
            alarm(0);
            error = 1;
            STOP = TRUE;
        }
        
        res = read(fd, &AUX_1, 1);
        if(AUX_1 == FLAG) flag_encontrada = 1;
        if(flag_encontrada) answer[i++] = AUX_1;
        if(AUX_1 == FLAG)   count++;
        if(count == 2){
            if(compara(answer, RR, 5)) break;
            if(compara(answer, REJ, 5)){
                alarm(0);
                stats_.num_REJ ++;
                flag_encontrada = 0;
                i = 0;
                res = write(fd, frame, new_bufSize);
                if(res == -1) return -1;
                numTries_Transmiter++;
                continue;
            }
            else{
                flag_encontrada = 1;
                i = 1;
                count = 1;
                continue;
            }
        }
    }
    alarm(0);

    if(error) return -1;

    return new_bufSize;
}



// Receive data in packet
int llread(char* packet){
    // controlo dos parametros
    if(!packet) return -1;

    /* Reeniciar timeouts */
    timeouts = 0;

    conta = 1;  //  PARA AS ESTATISTICAS

    /* Leitura */
    int res, STOP = FALSE, STATE = 0, count=0, packetSize = 0, flag_encontrada = 0;
    char AUX[255], AUX_1;
    char REJ[] = {FLAG, A_1, C_random1, BCC_1, FLAG};  //MAL OS COISOS
    char RR[] = {FLAG, A_1, C_random, BCC_1, FLAG};    //MAL OS COISOS
    char BCC_Check;

    /**/
    (void) signal(SIGALRM, timeout_);

    //printf("llread here... lets see what i receive\n");
    while (STOP==FALSE) /* loop for input */ 
    {   
        res = read(fd, &AUX_1, 1);
        if(AUX_1==FLAG) flag_encontrada=1;
        if(flag_encontrada) packet[packetSize++] = AUX_1;
        if(AUX_1 == FLAG)   count++;
        if(count == 2){

            stats_.num_bytes+=packetSize;

            BCC_Check = packet[packetSize-2];
            packetSize = resetFrameFormat(packet, packetSize);
            if(packetSize == -1)    return -1;
            packetSize = destuffing(packet, packetSize);
            if(packetSize == -1)    return -1; 
            
            //imprime(packet, packetSize);
            //printf("calBCC: 0x%02x check: 0x%02x\n", calculaBCC(packet, packetSize), BCC_Check);
                    // calculaBCC(packet, packetSize)==BCC_Check
                    
            if(calculaBCC(packet, packetSize) == BCC_Check){
                stats_.num_frames++;
                stats_.num_databytes+=packetSize;
                /*caso em que recebeu tudo direitinho*/
                //printf("Everything seems cool :) Sending ACK...\n");
                res = write(fd, RR, 5); //envia RR a confirmar que está pronto a receber o proximo pacote
                if(res == -1)   return -1;
                STOP = TRUE;
            }
            else{
                /*caso em que identificou que o pacote veio danificado*/
                //printf("Something went wrong here :( Sending REJ...\n");
                res = write(fd, REJ, 5);
                if(res == -1)   return -1;
                flag_encontrada = 0;
                packetSize = 0;
                count = 0;
                continue;
            }
        }
    }

    return packetSize;    
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics){

        /* Reeniciar timeouts */
    timeouts = 0, flag_ = 1;

    (void) signal(SIGALRM, timeout_);

    conta = 0;  //  PARA AS ESTATISTICAS
    if(TYPE == RECEIVER){
        //Espera o DISC
        int res, STOP = FALSE, STATE = 0, error = 0;
        char AUX[5], AUX_1;


        char UA[] = {FLAG, A_1, C_UA, BCC_2, FLAG}; // Definição do UA segundo o protocolo
        char DISC_rx[] = {FLAG, A_2, C_DISC, (A_2^C_DISC), FLAG};

        while(STOP == FALSE){

            if(timeouts < numTries_Receiver){
                if(flag_){
                    alarm(timeOut_Receiver);
                    flag_ = 0;
                }
            }
            else{
                alarm(0);
                STOP = TRUE; 
                error = 1;
                break;
            }
            res = read(fd, &AUX_1, 1);
            AUX[STATE] = AUX_1;
            switch (STATE)
            {
            case (STATE0):
                if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE1):
                if(AUX_1 == A_1) STATE = STATE2;
                else if(AUX_1 == FLAG) STATE = STATE0;
                else STATE = STATE0;
                break;
            case(STATE2):
                if(AUX_1 == C_DISC) STATE = STATE3;
                else if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE3):
                if(AUX_1 == (A_1^C_DISC)) STATE = STATE4;
                else if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE4):
                if(AUX_1 == FLAG) STATE = STATE5;
                else STATE = STATE0;
                break;
            }

            if(STATE == STATE5) STOP = TRUE;
        }
        alarm(0);

        if(error){
            printf("DISC NOT FOUND\n");
            return -1;
        } 
        
            printf("DISC (recebido): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", AUX[0], AUX[1], AUX[2], AUX[3], AUX[4]);

        //Envia DISC
        //sleep(15); // <-- Testes
        res = write(fd, DISC_rx, 5);
        if(res == -1) return -1;

            printf("DISC (enviado): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", DISC_rx[0], DISC_rx[1], DISC_rx[2], DISC_rx[3], DISC_rx[4]);

        /* RECEBE UA */
        STOP = FALSE, STATE = 0, timeouts = 0, flag_ = 1;
        while(STOP == FALSE){

            if(timeouts < numTries_Receiver){
                if(flag_){
                    alarm(timeOut_Receiver);
                    flag_ = 0;
                }
            }
            else{
                alarm(0);
                STOP = TRUE; 
                error = 1;
                break;
            }
            res = read(fd, &AUX_1, 1);
            AUX[STATE] = AUX_1;
            switch (STATE)
            {
            case (STATE0):
                if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE1):
                if(AUX_1 == A_1) STATE = STATE2;
                else if(AUX_1 == FLAG) STATE = STATE0;
                else STATE = STATE0;
                break;
            case(STATE2):
                if(AUX_1 == C_UA) STATE = STATE3;
                else if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE3):
                if(AUX_1 == (A_1^C_UA)) STATE = STATE4;
                else if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE4):
                if(AUX_1 == FLAG) STATE = STATE5;
                else STATE = STATE0;
                break;
            }

            if(STATE == STATE5) STOP = TRUE;
        }
        alarm(0);
        if(error){
            printf("UA NOT FOUND\n");
            return -1;
        } 
        printf("UA (recebido): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", AUX[0], AUX[1], AUX[2], AUX[3], AUX[4]);

        //mata-se
        res = closeSerialTerminal(fd);
        if(res == -1) return -1;
        if(showStatistics)  Statistics(stats_);
        return 1;
    }
    else if(TYPE == TRANSMITTER){

        int res, STOP = FALSE, STATE = 0, error = 0;
        char AUX[5], AUX_1;


        char UA[] = {FLAG, A_1, C_UA, (C_UA^A_1), FLAG}; // Definição do UA segundo o protocolo
        char DISC_tx[] = {FLAG, A_1, C_DISC, (A_1^C_DISC), FLAG};
        //Envia DISC
        //sleep(5); // <-- Testes
        res = write(fd, DISC_tx, 5);
        if(res == -1) return -1;

                    printf("DISC (enviado): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", DISC_tx[0],DISC_tx[1], DISC_tx[2], DISC_tx[3], DISC_tx[4]);

        //ESPERA DISC
        while(STOP == FALSE){
            if(timeouts < numTries_Transmiter){
                if(flag_){
                    alarm(timeOut_Transmiter);
                    flag_ = 0;
                }
            }
            else{
                alarm(0);
                STOP = TRUE; 
                error = 1;
                break;
            }

            res = read(fd, &AUX_1, 1);
            AUX[STATE] = AUX_1;
            switch (STATE)
            {
            case (STATE0):
                if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE1):
                if(AUX_1 == A_2) STATE = STATE2;
                else if(AUX_1 == FLAG) STATE = STATE0;
                else STATE = STATE0;
                break;
            case(STATE2):
                if(AUX_1 == C_DISC) STATE = STATE3;
                else if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE3):
                if(AUX_1 == (A_2^C_DISC)) STATE = STATE4;
                else if(AUX_1 == FLAG) STATE = STATE1;
                else STATE = STATE0;
                break;
            case(STATE4):
                if(AUX_1 == FLAG) STATE = STATE5;
                else STATE = STATE0;
                break;
            }

            if(STATE == STATE5) STOP = TRUE;
        }
        alarm(0);
        if(error){
            printf("DISC NOT FOUND\n");
            return -1;
        }
            printf("DISC (recebido): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", AUX[0], AUX[1], AUX[2], AUX[3], AUX[4]);

        // ENVIA UA
        //MATA-se
        res = write(fd, UA, 5);
        if(res == -1) return -1;
         
                printf("UA (Enviado): (0x%02X)(0x%02X)(0x%02X)(0x%02X)(0x%02X)\n", UA[0], UA[1], UA[2], UA[3], UA[4]);

        res = closeSerialTerminal(fd);
        if(res == -1) return -1;
        if(showStatistics)  Statistics(stats_);
        return 1;

    }
    return -1;
}