#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char BYTE;
typedef unsigned int UNIT;

#define ROL(i, W) ( ((W)<<(i)) | ((W)>>(32-i)) )
#define ROR(i, W) ( ((W)>>(i)) | ((W)<<(32-i)) )
#define BTOW(b1, b2, b3, b4) ( ((UNIT)b1<<24) | ((UNIT)b2<<16) | ((UNIT)b3<<8) | ((UNIT)b4) )

#define Nr 24
static const UNIT delta[8] = {
    0xC3EFE9DB, 0x44626B02, 0x79E27C8A, 0x78DF30EC,
    0x715EA49E, 0xC785DA0A, 0xE04EF22A, 0xE5C40957
};

void KeySchedule_128(BYTE *Key, UNIT *RoundKey);
void LEA_Encryption(UNIT *pText, UNIT *cText, UNIT *RoundKey);
void Enc_Round(UNIT *Out, UNIT *In, UNIT *RK);
void LEA_Decryption(UNIT *cText, UNIT *dcText, UNIT *RoundKey);
void Dec_Round(UNIT *Out, UNIT *In, UNIT *RK);

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

void LEA_Encryption(UNIT *pText, UNIT *cText, UNIT *RoundKey){
    int i, r;
    UNIT XIN[4] = {0,}; //32bits * 4 = 128bits
    UNIT XOUT[4] = {0,};

    //pText -> XIN
    for(i=0;i<4;i++){
        XIN[i] = pText[i];
    }

    //Round
    for(r=0;r<Nr;r++){
        Enc_Round(XOUT, XIN, (RoundKey+(r*6)));
        for(i=0;i<4;i++){
            XIN[i] = XOUT[i];
        }
    }

    //XOUT -> cText
    for(i=0;i<4;i++){
        cText[i] = XOUT[i];
    }
}

void Enc_Round(UNIT *Out, UNIT *In, UNIT *RK){
    Out[0] = ROL(9, ((In[0]^RK[0]) + (In[1]^RK[1])));
    Out[1] = ROL(5, ((In[1]^RK[2]) + (In[2]^RK[3])));
    Out[2] = ROL(3, ((In[2]^RK[4]) + (In[3]^RK[5])));
    Out[3] = In[0];
}

void LEA_Decryption(UNIT *cText, UNIT *dcText, UNIT *RoundKey){
    int i, r;
    UNIT XIN[4] = {0,};
    UNIT XOUT[4] = {0,};

    for(i=0;i<4;i++){
        XIN[i] = cText[i];
    }

    for(r=0;r<Nr;r++){
        Dec_Round(XOUT, XIN, RoundKey+((Nr-r-1)*6));
        for(i=0;i<4;i++){
            XIN[i] = XOUT[i];
        }
    }

    for(i=0;i<4;i++){
        dcText[i] = XOUT[i];
    }
}

void Dec_Round(UNIT *Out, UNIT *In, UNIT *RK){
    Out[0] = In[3];
    Out[1] = ((ROR(9, In[0])) - (Out[0]^RK[0]))^RK[1];
    Out[2] = ((ROR(5, In[1])) - (Out[1]^RK[2]))^RK[3];
    Out[3] = ((ROR(3, In[2])) - (Out[2]^RK[4]))^RK[5];
}

void main(){
    int i;
    UNIT KeyRound[6 * Nr] = {0,};
    UNIT pText[4] = {0x13579BDF, 0x02468ACE, 0xC0FFEE0, 0xAE472BE6}; //Plain Text
    UNIT cText[4] = {0,}; //Cipher Text
    UNIT dcText[4] = {0,}; //DeCipher Text

    BYTE Key[16] = {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF,
        0x13, 0x57, 0x9B, 0xEF,
        0x02, 0x46, 0x8A, 0xCE
    };

    //Key Generater
    KeySchedule_128(Key, KeyRound);

    printf("\n\nPlain Text: %08X %08X %08X %08X", pText[0], pText[1], pText[2], pText[3]);

    LEA_Encryption(pText, cText, KeyRound);
    printf("\n\nCipher Text: %08X %08X %08X %08X", cText[0], cText[1], cText[2], cText[3]);

    LEA_Decryption(cText, dcText, KeyRound);
    printf("\n\nDecipher Text: %08X %08X %08X %08X", dcText[0], dcText[1], dcText[2], dcText[3]);

}