#include "Printer.h"
#include <avr/pgmspace.h>
#include "ScanCode.h"

uint8_t Printer::decode3to8(int value){
    uint8_t temp = 0b10000000;
    int i = 0;
    while (i < value){
        temp = temp >> 1;
        i++;
    }
    return temp;             
}

uint16_t Printer::decode(uint8_t scancode){
    uint8_t col = decode3to8((scancode>>3)&0b00000111);
    uint8_t row = decode3to8(scancode&0b00000111);
    //uint8_t shift = scancode & 0b10000000;
    uint16_t modefier = 0x0000;
    if ((scancode&SFT_UP)!=0) modefier = 0x0080;
    return (col)|(row<<8)|(modefier);
}

Printer::Printer(
    uint8_t scanInterruptNum,
    int _codePin,
    int loadPin, 
    int _pringingPin,
    int feedingPin, 
    int waitingSOP, 
    int waitingSOF, 
    int waitingEOA
    ):
    scan_intr_num(scanInterruptNum),
    _code_pin(_codePin),
    load_pin(loadPin), 
    _printing_pin(_pringingPin),
    feeding_pin(feedingPin), 
    waiting_SOP_indicator(waitingSOP),
    waiting_SOF_indicator(waitingSOF),
    waiting_EOA_indicator(waitingEOA)
    {}

void Printer::begin() {
    was_CR = false;
    SPI.begin();
    LoadSwitch(0);
    digitalWrite(waiting_SOP_indicator,LOW);
    digitalWrite(waiting_SOF_indicator,LOW);
    digitalWrite(waiting_EOA_indicator,LOW);
}

void Printer::Wait4EOP(){
    digitalWrite(waiting_EOA_indicator,HIGH);
    while(digitalRead(_printing_pin)==LOW){}
    digitalWrite(waiting_EOA_indicator,LOW);
}

void Printer::Wait4SOP(){
    digitalWrite(waiting_SOP_indicator,HIGH);
    while(digitalRead(_printing_pin)==HIGH){}
    digitalWrite(waiting_SOP_indicator,LOW);
}

void Printer::Wait4SOF(){
    digitalWrite(waiting_SOF_indicator,HIGH);
    while(digitalRead(feeding_pin)==LOW){}
    digitalWrite(waiting_SOF_indicator,LOW);
}

void Printer::Wait4EOF(){
    digitalWrite(waiting_SOF_indicator,HIGH);
    while(digitalRead(feeding_pin)==HIGH){}
    digitalWrite(waiting_SOF_indicator,LOW);
}

void Printer::Print(uint8_t scancode) {
    EIMSK &= ~(1 << scan_intr_num);
    Wait4EOP();
    LoadSwitch(decode(scancode));
    Wait4SOP();
    LoadSwitch(0);
    EIFR |= (1 << scan_intr_num);   // clear pending
    EIMSK |= (1 << scan_intr_num);
}

void Printer::PrintCRLF() {
    EIMSK &= ~(1 << scan_intr_num);
    Wait4EOF();
    LoadSwitch(decode(KEY_RETURN));
    Wait4SOF();
    LoadSwitch(0);
    Wait4EOF();
    EIFR |= (1 << scan_intr_num);   // clear pending
    EIMSK |= (1 << scan_intr_num);
}

void Printer::PrintAccent(uint8_t scancode) {
    EIMSK &= ~(1 << scan_intr_num);
    Wait4EOF();
    LoadSwitch(decode(scancode));
    Wait4SOF();
    LoadSwitch(0);
    EIFR |= (1 << scan_intr_num);   // clear pending
    EIMSK |= (1 << scan_intr_num);
}

void Printer::ResetCodeMode() {
    if (digitalRead(_code_pin) == HIGH) return;
    EIMSK &= ~(1 << scan_intr_num);
    LoadSwitch(decode(KEY_CODE));
    while (digitalRead(_code_pin)==LOW){}
    LoadSwitch(0);
    EIFR |= (1 << scan_intr_num);   // clear pending
    EIMSK |= (1 << scan_intr_num);
}

void Printer::LoadSwitch(uint16_t data) {
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    digitalWrite(load_pin, LOW);
    SPI.transfer16(data);
    digitalWrite(load_pin, HIGH);
    SPI.endTransaction();
}

void Printer::type(uint8_t ascii)
{
    if (ascii == '\n'){
        if (was_CR){
            was_CR = false;
            return;
        }
        else{
            PrintCRLF();
            return;   
        }
    }

    if (ascii == '\r'){
        PrintCRLF();
        was_CR = true;
        return;
    }

    was_CR = false;

    if (ascii == '<'){
        Print(NUM_9|SFT_UP); //(
        Print(KEY_BS);          //BS
        Print(KEY_MINUS);       //-
        return;
    }

    if (ascii == '>'){
        Print(NUM_0|SFT_UP); //)
        Print(KEY_BS);          //BS
        Print(KEY_MINUS);       //-
        return;
    }

    if (ascii == '{'){
        Print(NUM_9|SFT_UP); //(
        Print(KEY_BS);          //BS
        Print(KEY_EQUAL);       //=
        return;
    }

    if (ascii == '}'){
        Print(NUM_0|SFT_UP); //)
        Print(KEY_BS);          //BS
        Print(KEY_EQUAL);       //=
        return;
    }

    if (ascii == '['){
        Print(NUM_9|SFT_UP); //(
        Print(KEY_BS);          //BS
        Print(KEY_MINUS|SFT_UP); //_
        return;
    }
    if (ascii == ']'){
        Print(NUM_0|SFT_UP); //)
        Print(KEY_BS);          //BS
        Print(KEY_MINUS|SFT_UP); //_
        return;
    }

    if (ascii == '|'){
        Print(NUM_9|SFT_UP); //(
        Print(KEY_BS);          //BS
        Print(NUM_0|SFT_UP); //)
        return;
    }

    if (ascii == '^'){
        PrintAccent(KEY_CIRCUMFLEX); //^(circumflex accent)  
        Print(KEY_SPACE); //SPC
        return;
    }

    if (ascii == '`'){
        PrintAccent(KEY_GRAVE); //^(circumflex accent)  
        Print(KEY_SPACE); //SPC
        return;
    }

    uint8_t scanCode = pgm_read_word(&ascii_to_scancode_map[ascii]);
    if (scanCode!=KEY_NOP) Print(scanCode);
    return; 
}

const uint8_t Printer::ascii_to_scancode_map[] PROGMEM = {
    /*00*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP, 
    /*04*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP,
    /*08*/  KEY_BS,             KEY_TAB,            KEY_NOP,            KEY_NOP,  
    /*0c*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP,
    /*10*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP,
    /*14*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP,
    /*18*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP,
    /*1c*/  KEY_NOP,            KEY_NOP,            KEY_NOP,            KEY_NOP,
    /*20*/  KEY_SPACE,          NUM_1|SFT_UP,        KEY_QUOTE|SFT_UP,    NUM_3|SFT_UP,    
    /*24*/  NUM_4|SFT_UP,    NUM_5|SFT_UP,    NUM_7|SFT_UP,    KEY_QUOTE,  
    /*28*/  NUM_9|SFT_UP,    NUM_0|SFT_UP,    KEY_DEGREE|SFT_UP,   KEY_EQUAL|SFT_UP,
    /*2c*/  KEY_COMMA,          KEY_MINUS,          KEY_PERIOD,         KEY_SLASH,
    /*30*/  NUM_0,          NUM_1,          NUM_2,          NUM_3,
    /*34*/  NUM_4,          NUM_5,          NUM_6,          NUM_7,
    /*38*/  NUM_8,          NUM_9,          KEY_SEMICOL|SFT_UP,  KEY_SEMICOL,
    /*3c*/  KEY_NOP,            KEY_EQUAL,          KEY_NOP,            KEY_SLASH|SFT_UP, 
    /*40*/  NUM_2|SFT_UP,    ALPH_A|SFT_UP,   ALPH_B|SFT_UP,   ALPH_C|SFT_UP,
    /*44*/  ALPH_D|SFT_UP,   ALPH_E|SFT_UP,   ALPH_F|SFT_UP,   ALPH_G|SFT_UP,   
    /*48*/  ALPH_H|SFT_UP,   ALPH_I|SFT_UP,   ALPH_J|SFT_UP,   ALPH_K|SFT_UP,   
    /*4c*/  ALPH_L|SFT_UP,   ALPH_M|SFT_UP,   ALPH_N|SFT_UP,   ALPH_O|SFT_UP,
    /*50*/  ALPH_P|SFT_UP,   ALPH_Q|SFT_UP,   ALPH_R|SFT_UP,   ALPH_S|SFT_UP,  
    /*54*/  ALPH_T|SFT_UP,   ALPH_U|SFT_UP,   ALPH_V|SFT_UP,   ALPH_W|SFT_UP,
    /*58*/  ALPH_X|SFT_UP,   ALPH_Y|SFT_UP,   ALPH_Z|SFT_UP,   KEY_NOP,
    /*5c*/  NUM_6|SFT_UP,    KEY_NOP,            KEY_NOP,            KEY_MINUS|SFT_UP,
    /*60*/  KEY_NOP,            ALPH_A,         ALPH_B,         ALPH_C,
    /*64*/  ALPH_D,         ALPH_E,         ALPH_F,         ALPH_G,
    /*68*/  ALPH_H,         ALPH_I,         ALPH_J,         ALPH_K,
    /*6c*/  ALPH_L,         ALPH_M,         ALPH_N,         ALPH_O,
    /*70*/  ALPH_P,         ALPH_Q,         ALPH_R,         ALPH_S,
    /*74*/  ALPH_T,         ALPH_U,         ALPH_V,         ALPH_W,
    /*78*/  ALPH_X,         ALPH_Y,         ALPH_Z,         KEY_NOP,
    /*7c*/  KEY_NOP,            KEY_NOP,            KEY_DEGREE,         KEY_DEL,
};

