#include "Listner.h"
#include "Printer.h"
#include "AsciiCode.h"
#include "ScanCode.h"

Listner::Listner(Printer* prt, int readPin, int _capsPin, int _codePin)
    : _read_pin(readPin), _caps_pin(_capsPin), _code_pin(_codePin), printer(prt) {}

void Listner::begin() {

        code_d2 = HIGH;
        code_d1 = HIGH;
        code_d0 = HIGH;;

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
    
    if (col == 0) {
        code_d2 = code_d1;
        code_d1 = code_d0;
        code_d0 = digitalRead(_code_pin);
    }
    sense_d2[col] = sense_d1[col]; 
    sense_d1[col] = sense_d0[col];
    sense_d0[col] = value >> 8;
    uint8_t temp = sense_d2[col] & ~sense_d1[col] & ~sense_d0[col];
    pressed[col]|= temp;
    if (temp!=0) {
        isCodeMode = (code_d2==LOW);      
    }
}

Listner::ScanCode Listner::getScanCode(){
    Listner::ScanCode result = { KEY_NOP, false };
    for (int col = 0 ; col < 8 ; col++){
        uint8_t row_pattern = pressed[col]; 
        if (row_pattern){
            pressed[col] = KEY_NOP;
            result.isCodeMode = isCodeMode; isCodeMode = false;
            result.KeyCode = (col << 3) | (encode8to3(row_pattern) & 0b111);
            return result;
        }
    }
    return result;
}

bool Listner::isAllOff(){
    for (int col = 1 ; col < 8 ; col++){   /*col=0はスライドSWがつながっているのでチェックしない*/
        if (sense_d0[col] != 0xFF) return false;
    }
    return true;
}

void Listner::dumpSwitch(){
    Serial.print("\nsw_dump:");
    for (int i = 0 ; i < 8 ; i++ ){
        Serial.print(sense_d0[i],HEX);
    }
}

bool Listner::isUpperCase(){
    return (~sense_d0[0]&0x40)||(digitalRead(_caps_pin)==0);
}

int Listner::getAsciiCode(){
    Listner::ScanCode scan_code = getScanCode();
    if (scan_code.KeyCode == KEY_NOP) return -1;
    if (scan_code.isCodeMode){
        if (scan_code.KeyCode==ALPH_J) {
            printer->type('<');
            return '<';
        }
        if (scan_code.KeyCode==ALPH_K){
            printer->type('>');
            return '>';
        }
        if (scan_code.KeyCode==ALPH_L){
            if (isUpperCase()){
                printer->type('{');
                return '{';
            } 
            else{
                printer->type('[');
                return '[';
            }
        }
        if (scan_code.KeyCode==ALPH_M){
            if (isUpperCase()){
                printer->type('}');
                return '}';
            } 
            else{
                printer->type(']');
                return ']';
            }
        }
        if (scan_code.KeyCode==ALPH_B) {
            printer->Wait4SOP();
            return ASC_STX;
        }
        if (scan_code.KeyCode==ALPH_C){
            while (isAllOff()==0){}
            return ASC_ETX;
        } 

        if (scan_code.KeyCode==ALPH_D){
            printer->ResetCodeMode();
            return ASC_EOT;
        } 

        if (scan_code.KeyCode==ALPH_E){
            printer->ResetCodeMode();
            return ASC_ENQ;
        } 
   
        if (scan_code.KeyCode==ALPH_F){
            printer->ResetCodeMode();
            return ASC_ESC;
        }
       return -1;
    }
 
    int ascii;
    if (isUpperCase()) ascii = pgm_read_byte(&scan2ascii_upper[scan_code.KeyCode & 0x3f]);
    else ascii = pgm_read_byte(&scan2ascii_lower[scan_code.KeyCode & 0x3f]);

    if (ascii==ASC_NUL) return ASC_NUL;
    
    if ((ascii==ASC_CR)||(ascii==ASC_LF))printer->Wait4SOF();
    else printer->Wait4SOP();
    
    return ascii;
}

int encode8to3(uint8_t value){
    uint8_t mask = 0b10000000;
    for (int code = 0 ; code < 8 ; code++){
        if (value & mask) return code;
        mask = mask >> 1;
    }
    return -1;  
};

const char Listner::scan2ascii_lower[] PROGMEM = {
    /*00*/  ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,
    /*08*/  'a',        'b',        'c',        'd',        'e',        'f',        'g',        'h',
    /*10*/  'i',        'j',        'k',        'l',        'm',        'n',        'o',        'p',
    /*18*/  'q',        'r',        's',        't',        'u',        'v',        'w',        'x',
    /*20*/  'y',        'z',        '0',        '1',        '2',        '3',        '4',        '5',
    /*28*/  '6',        '7',        '8',        '9',        ',',        '\'',       ';',        '.',
    /*30*/  '/',        ASC_NUL,    '-',        '=',        ASC_NUL,    ASC_NUL,    '~',        ASC_SPC,
    /*38*/  ASC_HT,     ASC_DEL,    ASC_CR,     ASC_NUL,    ASC_BS,     ASC_NUL,    ASC_NUL,    ASC_NUL,
};

const char Listner::scan2ascii_upper[] PROGMEM = {
    /*40*/  ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,    ASC_NUL,
    /*48*/  'A',        'B',        'C',        'D',        'E',        'F',        'G',        'H',
    /*50*/  'I',        'J',        'K',        'L',        'M',        'N',        'O',        'P',
    /*58*/  'Q',        'R',        'S',        'T',        'U',        'V',        'W',        'X',
    /*60*/  'Y',        'Z',        ')',        '!',        '@',        '#',        '&',        '%',
    /*68*/  '\\',       '&',        -1,         '(',        ',',        '"',        ':',        '.',
    /*70*/  '?',        ASC_NUL,    '_',        '+',        ASC_NUL,    ASC_NUL,    '*',        ASC_SPC,
    /*78*/  ASC_HT,     ASC_DEL,    ASC_LF,     ASC_NUL,    ASC_BS,     ASC_NUL,    ASC_NUL,    ASC_NUL,
};
