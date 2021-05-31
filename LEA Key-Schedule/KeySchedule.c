#include <stdio.h>
#include <stdio.h>
#include <string.h>

#define Nr 24 //LEA-128은 24라운드로 구성됨

typedef unsigned char BYTE;
typedef unsigned int UNIT;

static const UNIT delta[8] = {
    0xC3EFE9DB, 0x44626B02, 0x79E27C8A, 0x78DF30EC,
    0x715EA49E, 0xC785DA0A, 0xE04EF22A, 0xE5C40957
};

#define ROL(i, W) ( ((W)<<(i)) | ((W)>>(32-(i))) )
#define ROR(i, W) ( ((W)>>(i)) | ((W)<<(32-(i))) )
#define BTOW(b1, b2, b3, b4) ( ((UNIT)b1<<24)|((UNIT)b2<<16)|((UNIT)b3<<8)|((UNIT)b4) )

void KeySchedule_128(BYTE *Key, UNIT *RoundKey);

void KeySchedule_128(BYTE *Key, UNIT *RoundKey){
    int i;
    UNIT T[4] = {0,};
    
    for(i=0;i<4;i++){
        T[i] = BTOW(Key[i*4+3], Key[i*4+2], Key[i*4+1], Key[i*4]);
    }

    for(i=0;i<24;i++){
        T[0] = ROL(1,  (T[0]+ROL(i  , delta[i%4]))); //T[0] = ROL(1, Addition(T[0], ROL(i, delta[i%4])))
        T[1] = ROL(3,  (T[1]+ROL(i+1, delta[i%4]))); //T[1] = ROL(3, Addition(T[1], ROL(i+1, delta[i%4])))
        T[2] = ROL(6,  (T[2]+ROL(i+2, delta[i%4]))); //T[2] = ROL(6, Addition(T[2], ROL(i+2, delta[i%4])))
        T[3] = ROL(11, (T[3]+ROL(i+3, delta[i%4]))); //T[3] = ROL(11, Addition(T[3], ROL(i+3, delta[i%4])))

        //RKenc = T[0], T[1], T[2], T[1], T[3], T[1]
        RoundKey[i*6+0] = T[0];
        RoundKey[i*6+1] = T[1];
        RoundKey[i*6+2] = T[2];
        RoundKey[i*6+3] = T[1];
        RoundKey[i*6+4] = T[3];
        RoundKey[i*6+5] = T[1];
    }
}

void main(){
    int i, r;
    UNIT RoundKey[6*Nr] = {0,};

    BYTE Key[16] = {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF,
        0x13, 0x57, 0x9B, 0xEF,
        0x02, 0x46, 0x8A, 0xCE
    };

    KeySchedule_128(Key, RoundKey);

    printf("\nKey: ");
    for(i=0;i<16;i++){
        if(i%4==0) printf("\n");
        printf("%02X ", Key[i]);
    }
    printf("\n");

    for(r=0;r<Nr;r++){
        printf("\nRound[%02d]: ", r);
        for(i=0;i<6;i++){
            printf("%08X ", RoundKey[r*6+i]);
        }
    }
}