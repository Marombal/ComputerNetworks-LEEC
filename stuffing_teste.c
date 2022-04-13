#include <string.h>
#include <stdio.h>

#define FLAG 0x7e
#define A_1 0x03
#define A_2 0x01
#define C 0x03
#define ESC 0x7d
#define XOR 0x20

int stuffing(char* buf, int size);

int main(){
    char buf[50] = "123~123"; // 
    int res;
    printf("%ld\nInit_Char: %s\n",(strlen(buf)), buf);
    printf("Init_Hex:");
    for(int i = 0; i < strlen(buf); i++){
        printf(" 0x%02X ", buf[i]);
    }
    printf("\n");
    res = stuffing(buf, (strlen(buf)));
    printf("%ld\nInit_Char: %s\n",(strlen(buf)), buf);
    printf("Init_Hex:");
    for(int i = 0; i < strlen(buf); i++){
        printf(" 0x%02X ", buf[i]);
    }
    printf("\n");
}

int stuffing(char* buf, int size){
    if((!buf)||(size<0)) return -1;
    int act_size = size;
    char aux1, aux2;

    for(int i = 0; i < size; i++){
        if(buf[i] == ESC){
            size++;
            aux1 = buf[i+1];
            buf[i+1] = ESC^XOR;    //this should be 0x5e
            for(int j = i+2; j <= size ;j++){
                aux2 = buf[j];
                buf[j]=aux1;
                aux1 = aux2;
            }
        }
    }

    for(int i = 0; i<size; i++){
        if(buf[i] == FLAG){
            buf[i]=ESC;
            size++;
            aux1=buf[i+1];
            buf[i+1]=FLAG^XOR;  //this should be 
            for(int j = i+2; j<=size; j++){
                aux2 = buf[j];
                buf[j]=aux1;
                aux1=aux2;
            }
        }
    }
    return size;
}
