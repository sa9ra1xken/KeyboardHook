#include "Typer.h"
#include <avr/pgmspace.h>

Typer::Typer(
    int loadPin, 
    int _pringingPin,
    int feedingPin, 
    int waitingSOP, 
    int waitingSOF, 
    int waitingEOA
    ):
    load_pin(loadPin), 
    _printing_pin(_pringingPin),
    feeding_pin(feedingPin), 
    waiting_SOP_indicator(waitingSOP),
    waiting_SOF_indicator(waitingSOF),
    waiting_EOA_indicator(waitingEOA)
    {}

void Typer::begin() {
    was_CR = false;
    SPI.begin();
    LoadSwitch(0);
    digitalWrite(waiting_SOP_indicator,LOW);
    digitalWrite(waiting_SOF_indicator,LOW);
    digitalWrite(waiting_EOA_indicator,LOW);
}

void Typer::PushAndRelease(uint16_t data) {
    LoadSwitch(data);
    delay(30);    
    LoadSwitch(0);
}    

void Typer::Wait4EOP(){
    digitalWrite(waiting_EOA_indicator,HIGH);
    while(digitalRead(_printing_pin)==LOW){}
    digitalWrite(waiting_EOA_indicator,LOW);
}

void Typer::Wait4SOP(){
    digitalWrite(waiting_SOP_indicator,HIGH);
    while(digitalRead(_printing_pin)==HIGH){}
    digitalWrite(waiting_SOP_indicator,LOW);
}

void Typer::Wait4SOF(){
    digitalWrite(waiting_SOF_indicator,HIGH);
    while(digitalRead(feeding_pin)==LOW){}
    digitalWrite(waiting_SOF_indicator,LOW);
}

void Typer::Wait4EOF(){
    digitalWrite(waiting_SOF_indicator,HIGH);
    while(digitalRead(feeding_pin)==HIGH){}
    digitalWrite(waiting_SOF_indicator,LOW);
}

void Typer::Print(uint16_t data) {
    Wait4EOP();
    LoadSwitch(data);
    Wait4SOP();
    LoadSwitch(0);
}

void Typer::LineFeed() {
    Wait4EOF();
    LoadSwitch(0x2001/*LF*/);
    Wait4SOF();
    LoadSwitch(0);
    Wait4EOF();
}

void Typer::PrintAccent(uint16_t data) {
    Wait4EOF();
    LoadSwitch(data);
    Wait4SOF();
    LoadSwitch(0);
}

void Typer::LoadSwitch(uint16_t data) {
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    digitalWrite(load_pin, LOW);
    SPI.transfer16(data);
    digitalWrite(load_pin, HIGH);
    SPI.endTransaction();
}

void Typer::type(uint8_t code)
{
    if (code == '\n'){
        if (was_CR){
            was_CR = false;
            return;
        }
        else{
            LineFeed();
            return;   
        }
    }

    if (code == '\r'){
        LineFeed();
        was_CR = true;
        return;
    }

    was_CR = false;

    if (code == '<'){
        Print(0x1084); //(
        Print(0x0801); //BS
        Print(0x2002); //-
        return;
    }

    if (code == '='){
        Print(0x1002); //=
        return;
    }

    if (code == '>'){
        Print(0x2088); //)
        Print(0x0801); //BS
        Print(0x2002); //-
        return;
    }

    if (code == '{'){
        Print(0x1084); //(
        Print(0x0801); //BS
        Print(0x1002); //=
        return;
    }

    if (code == '|'){
        Print(0x1084); //(
        Print(0x0801); //BS
        Print(0x2088); //)
        return;
    }

    if (code == '}'){
        Print(0x2088); //)
        Print(0x0801); //BS
        Print(0x1002); //=
        return;
    }

    if (code == '['){
        Print(0x1084); //(
        Print(0x0801); //BS
        Print(0x2082); //_
        return;
    }

    if (code == '\\'){
        Print(0x8084); //\
        return;
    }

    if (code == ']'){
        Print(0x2088); //)
        Print(0x0801); //BS
        Print(0x2082); //_
        return;
    }

    if (code == '^'){
        PrintAccent(0x4002); //^(circumflex accent)  
        Print(0x0102); //SPC
        return;
    }

    if (code == '~'){
        Print(0x0202); //~
        return;
    }

    if (code >= key_map_carriage_top){
        uint8_t index = code - key_map_carriage_top;
        if (index < key_map_carriage_size){
            uint16_t pat = pgm_read_word(&key_map_carriage[index]);
            Print(pat);
            return;
        } 
    }

    if (code >= key_map_numeric_top){
        uint8_t index = code - key_map_numeric_top;
        if (index < key_map_numeric_size){
            uint16_t pat = pgm_read_word(&key_map_numeric[index]);
            Print(pat);
            return;
        } 
    }
    if (code >= key_map_ALPHA_top){
        uint8_t index = code - key_map_ALPHA_top;
        if (index < key_map_ALPHA_size){
            uint16_t pat = pgm_read_word(&key_map_ALPHA[index]);
            Print(pat);
            return;
        } 
    } 
    if (code >= key_map_alpha_top){
        uint8_t index = code - key_map_alpha_top;
        if (index < key_map_alpha_size){
            uint16_t pat = pgm_read_word(&key_map_alpha[index]);
            Print(pat);
            return;
        } 
    }
    return; 
}

const uint16_t Typer::key_map_carriage[] PROGMEM = {
    0x0801, //BS
    0x8001, //HT
};
const uint8_t Typer::key_map_carriage_top = '\b';
const uint8_t Typer::key_map_carriage_size =
    sizeof(Typer::key_map_carriage) / sizeof(Typer::key_map_carriage[0]);

const uint16_t Typer::key_map_numeric[] PROGMEM = {
    0x0102, //SP
    0x1088, //!
    0x0484, //"
    0x0488, //#
    0x0288, //$
    0x0188, //%
    0x4084, //&
    0x0404, //'
    0x1084, //(
    0x2088, //)
    0x0282, //*
    0x1082, //+
    0x0804, //,
    0x2002, //-
    0x0104, //.
    0x8002, ///
    0x2008, //0
    0x1008, //1
    0x0808, //2
    0x0408, //3
    0x0208, //4
    0x0108, //5
    0x8004, //6
    0x4004, //7
    0x2004, //8
    0x1004, //9
    0x0284, //:
    0x0204, //;
};
const uint8_t Typer::key_map_numeric_top = ' ';
const uint8_t Typer::key_map_numeric_size =
    sizeof(Typer::key_map_numeric) / sizeof(Typer::key_map_numeric[0]);

const uint16_t Typer::key_map_ALPHA[] PROGMEM = {
    0x8002,	//?
    0x0888,	//@
    0x80C0, //A
    0x40C0, //B
    0x20C0, //C
    0x10C0, //D
    0x08C0, //E
    0x04C0, //F
    0x02C0, //G
    0x01C0, //H
    0x80A0, //I
    0x40A0, //J
    0x20A0, //K
    0x10A0, //L
    0x08A0, //M
    0x04A0, //N
    0x02A0, //O
    0x01A0, //P
    0x8090, //Q
    0x4090, //R
    0x2090, //S
    0x1090, //T
    0x0890, //U
    0x0490, //V
    0x0290, //W
    0x0190, //X
    0x8088, //Y
    0x4088, //Z
};
const uint8_t Typer::key_map_ALPHA_top = '?';
const uint8_t Typer::key_map_ALPHA_size =
    sizeof(Typer::key_map_ALPHA) / sizeof(Typer::key_map_ALPHA[0]);

const uint16_t Typer::key_map_alpha[] PROGMEM = {
    0x2082, //_
    0x0404, //`
    0x8040, //a
    0x4040, //b
    0x2040, //c
    0x1040, //d
    0x0840, //e
    0x0440, //f
    0x0240, //g
    0x0140, //h
    0x8020, //i
    0x4020, //j
    0x2020, //k
    0x1020, //l
    0x0820, //m
    0x0420, //n
    0x0220, //o
    0x0120, //p
    0x8010, //q
    0x4010, //r
    0x2010, //s
    0x1010, //t
    0x0810, //u
    0x0410, //v
    0x0210, //w
    0x0110, //x
    0x8008, //y
    0x4008, //z
};
const uint8_t Typer::key_map_alpha_top = '_';
const uint8_t Typer::key_map_alpha_size =
    sizeof(Typer::key_map_alpha) / sizeof(Typer::key_map_alpha[0]);

uint8_t Typer::decode3to8(int value){
    uint8_t temp = 0b10000000;
    int i = 0;
    while (i < value){
        temp = temp >> 1;
    }
    return temp;             
}