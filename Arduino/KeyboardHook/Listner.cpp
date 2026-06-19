#include "Listner.h"
#include "ASCII_CTR.h"

Listner::Listner(int readPin, int _capsPin, int _codePin)
    : _read_pin(readPin), _caps_pin(_capsPin), _code_pin(_codePin) {}

void Listner::begin() {
    for (int i = 0 ; i < 8 ; i++){
        sense_d1[i]= 0xff;
        sense_d2[i]= 0xff;
        pressed[i] = 0x00;
    } 
    digitalWrite(_read_pin, HIGH);
    SPI.begin();
}

void Listner::onScan(){
    digitalWrite(_read_pin, LOW);
    delayMicroseconds(1);
    digitalWrite(_read_pin, HIGH);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    uint16_t value = SPI.transfer16(0x0000);
    SPI.endTransaction();
    int col = encode8to3(~value & 0xff);
    if (col < 0) return;
    sense_d2[col] = sense_d1[col]; 
    sense_d1[col] = sense_d0[col];
    sense_d0[col] = value >> 8;
    pressed[col]|= sense_d2[col] & ~sense_d1[col] & ~sense_d0[col]; 
}

int Listner::getScanCode(){
    for (int col = 0 ; col < 8 ; col++){
        uint8_t temp = pressed[col]; 
        if (temp){
            int row = encode8to3(temp);
            pressed[col]=0;
            return (col << 3) | (encode8to3(temp) & 0b111);
        }
    }
    return -1; //no key has been pressed.
}

bool Listner::isAllOff(){
    for (int col = 1 ; col < 8 ; col++){   /*COL1はスライドSWがつながっているのでチェックしない*/
        if (sense_d2[col] != 0xFF) return false;
        if (sense_d1[col] != 0xFF) return false;
        if (sense_d0[col] != 0xFF) return false;
    }
    return true;
}

bool Listner::isUpperCase(){
    return (~sense_d0[0]&0x40)||(digitalRead(_caps_pin)==0);
}

int Listner::getAsciiCode(){
    int scan_code = getScanCode();
    if (scan_code < 0) return -1;

    if ((digitalRead(_code_pin)==0)){
        if (scan_code==0x11) return '<';
        if (scan_code==0x12) return '>';
        if (scan_code==0x13){
            if (isUpperCase()) return '{';
            else return '[';
        }
        if (scan_code==0x14){
            if (isUpperCase()) return '}';
            else return ']';
        }
        if (scan_code==0x09) return STX; //Bキー　
        if (scan_code==0x0A) return ETX; //Cキー
        if (scan_code==0x0C) return ENQ; //Eキー
        if (scan_code==0x0D) return ESC; //Fキー
    }
    else{
        if (scan_code==0x4001) return DEL;
        if (isUpperCase()) scan_code |= 0x40;
        return pgm_read_byte(&scan2ascii[scan_code]);
    }
}

int encode8to3(uint8_t value){
    uint8_t mask = 0b10000000;
    for (int code = 0 ; code < 8 ; code++){
        if (value & mask) return code;
        mask = mask >> 1;
    }
    return -1;  
};

//#define SPC 0x20
//#define NUL 0x00
//#define LF  0x0A
//#define CR  0x0D

const char Listner::scan2ascii[] PROGMEM = {
    /*00*/  NUL,     NUL,   NUL,    NUL,    NUL,    NUL,    NUL,    NUL,
    /*08*/  'a',    'b',    'c',    'd',    'e',    'f',    'g',    'h',
    /*10*/  'i',    'j',    'k',    'l',    'm',    'n',    'o',    'p',
    /*18*/  'q',    'r',    's',    't',    'u',    'v',    'w',    'x',
    /*20*/  'y',    'z',    '0',    '1',    '2',    '3',    '4',    '5',
    /*28*/  '6',    '7',    '8',    '9',    ',',    '\'',   ';',    '.',
    /*30*/  '/',    NUL,    '-',    '=',    NUL,    NUL,    '~',    SPC,
    /*38*/  HT,     DEL,    CR,     NUL,    BS,     NUL,    NUL,    NUL,
    /*40*/  NUL,    NUL,    NUL,    NUL,    NUL,    NUL,    NUL,    NUL,
    /*48*/  'A',    'B',    'C',    'D',    'E',    'F',    'G',    'H',
    /*50*/  'I',    'J',    'K',    'L',    'M',    'N',    'O',    'P',
    /*58*/  'Q',    'R',    'S',    'T',    'U',    'V',    'W',    'X',
    /*60*/  'Y',    'Z',    ')',    '!',    '@',    '#',    '&',    '%',
    /*68*/  '\\',   '&',    -1,     '(',    ',',    '"',    ':',    '.',
    /*70*/  '?',    NUL,    '_',    '+',    NUL,    NUL,    '*',    SPC,
    /*78*/  HT,     DEL,    LF,     NUL,    BS,     NUL,    NUL,    NUL,
};
