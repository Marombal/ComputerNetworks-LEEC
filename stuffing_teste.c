#include <string.h>
#include <stdio.h>

#define FLAG 0x7e
#define A_1 0x03
#define A_2 0x01
#define C 0x03
#define BCC_1 (A_1^C)
#define BCC_2 (A_2^C)
#define ESC 0x7d
#define XOR 0x20

#define TAM_MAX 50

int stuffing(char* buf, int size);
int destuffing(char* buf, int size);
int setupFrameFormat(char* buf, char* frame, int size);
int resetFrameFormat(char* buf, int size);
void imprime(char* buf, int size);

int main(){
    char buf[50] = "123~12}}3"; // strlen-1 = 7
    int res, res1;
    char frame[TAM_MAX];
    
    printf("Start: ");
    imprime(buf, strlen(buf));

    res = setupFrameFormat(buf, frame, strlen(buf));

    printf("sFrame: ");
    imprime(frame, res);
    
    res = stuffing(frame, res);
    printf("Stuff: ");   
    imprime(frame, res);

    res = destuffing(frame, res);
    printf("dStuf: ");
    imprime(frame, res);

    res = resetFrameFormat(frame, res);
    printf("Final: ");
    imprime(frame, res);

    for(int i = 0; i < res; i++){
        if(buf[i]!=frame[i]){
            printf("\nERROR. Strat!=Final\n");
            return -1;
        }
    }
    printf("\nSUCESS (START = FINAL, as expected)\n\n");
}


int stuffing(char* buf, int size){
    if((!buf)||(size<0)) return -1;
    
    char aux1, aux2;

    for(int i = 3; i < size; i++){
        if(buf[i] == ESC){
            size++;
            aux1 = buf[i+1];
            buf[i+1] = (ESC^XOR);    //this should be 0x5e
            for(int j = i+2; j <= size ;j++){
                aux2 = buf[j];
                buf[j]=aux1;
                aux1 = aux2;
            }
        }
    }

    for(int i = 3; i<size; i++){
        if(buf[i] == FLAG){
            buf[i]=ESC;
            size++;
            aux1=buf[i+1];
            buf[i+1]=(FLAG^XOR);  //this should be 
            for(int j = i+2; j<=size; j++){
                aux2 = buf[j];
                buf[j]=aux1;
                aux1=aux2;
            }
        }
    }
    return size;
}

int destuffing(char* buf, int size){
    if((!buf)||(size<0)) return -1;

    for(int i = 0; i < size; i++){
        if(buf[i] == ESC){
            if(buf[i+1] == (FLAG^XOR)){//printf("a%0X", buf[i+1]);
                for(int j = i; j < size; j++){                
                    buf[j] = buf[j+1];
                }
                size--;
                buf[i] = FLAG;
            }
            else if(buf[i+1] == (ESC^XOR)){//printf("b%0X", buf[i+1]);
                for(int j = i; j < size; j++){
                    buf[j] = buf[j+1];
                }
                size--;
                buf[i] = ESC;
            }
            else{
                return -1;}
        }
    }
    return size;
}

int setupFrameFormat(char* buf, char* frame, int size){
    if((!buf)||(size<0)) return -1;
    //char frame[TAM_MAX];
    frame[0] = FLAG;
    frame[1] = A_1;
    frame[2] = C;
    frame[3] = BCC_1;
    for(int i = 0; i < size; i++){
        frame[i+4] = buf[i];
    }
    frame[size + 4] = BCC_2;
    frame[size + 5] = FLAG;

    //imprime(frame, size+6);
    return size+6;
}


int resetFrameFormat(char* buf, int size){
    if((!buf)||(size<0)) return -1;
    /* Tirar a FLAG, A, C, BCC do inicio do buf */
    for(int i = 0; i < size; i++){
        buf[i] = buf[i+4];
    }
    size = size - 4;
    /* Tirar o final da */
    size = size - 2;

    return size;
}


void imprime(char* buf, int size){
    if((!buf)||(size < 0)) return;
    //printf("Init_Hex:");
    for(int i = 0; i < size; i++){
        printf(" 0x%02X ", buf[i]);
    }
    printf("\n");
    return;
}
