//#define CHECK_ASCII_CODE
//#define CHECK_SCAN_CODE
//#define USE_CTS
#define USE_ACK

#include "SpiBus.h"
#include "Listner.h"
#include "Typer.h"
#include <MsTimer2.h>
#include "ASCII_CTR.h"

const int scanTimingPin = 2;
const int _printingPin = 3;
const int inputLatchPin = 4;
const int _capsPin = 5;
const int _ctsPin = 6;
const int feedingPin = 7;
const int outputLatchPin = 9;  
const int _codePin = A0;

const int Waiting4SOP = A1;
const int Waiting4SOF = A2;
const int Waiting4EOA = A3;

const int XOFF = 0x13;
const int XON = 0x11;
const int ACK = 0x06;

Listner listner(inputLatchPin, _capsPin, _codePin);
Typer typer(outputLatchPin, _printingPin, feedingPin, Waiting4SOP, Waiting4SOF, Waiting4EOA);

uint8_t scan_intr;

void setup() {
    
    pinMode(scanTimingPin, INPUT_PULLUP);
    pinMode(_ctsPin, OUTPUT);
    
    pinMode(outputLatchPin, OUTPUT);
    pinMode(_printingPin, INPUT_PULLUP);
    pinMode(feedingPin, INPUT_PULLUP);

    pinMode(inputLatchPin, OUTPUT);
    pinMode(_capsPin, INPUT_PULLUP);
    pinMode(_codePin, INPUT_PULLUP);
    
    scan_intr = digitalPinToInterrupt(scanTimingPin);     

    Serial.begin(9600);
    while (!Serial) { }

    SpiBus::begin();
    listner.begin();
    typer.begin();
#ifdef USE_CTS    
    digitalWrite(_ctsPin,LOW);
#endif     
    attachInterrupt(scan_intr, onScanISR, RISING);
}

void onScanISR() {
    listner.onScan();
}

void localEcho(int code){
    detachInterrupt(scan_intr);
    typer.type(code);
    attachInterrupt(scan_intr, onScanISR, RISING);
}


void loop() {
    if (Serial.available() > 0) {        
        detachInterrupt(scan_intr);
        if ((digitalRead(_codePin)==HIGH)&&listner.isAllOff()){
            //while (Serial.available() > 0) 
            //{
                typer.type(Serial.read());
                Serial.write(ACK);    
            //}
        }
        attachInterrupt(scan_intr, onScanISR, RISING);
    }
    int code = listner.getAsciiCode();
    if (code>NUL){
        if ((code=='<')||(code=='>')||(code=='[')||(code==']')||(code=='{')||(code=='}')){
            localEcho(code);
            Serial.write(code);
        }
        else if ((code==CR)||(code==LF)){
            typer.Wait4SOF();
            Serial.write(code);
        }
        else if (code==STX){
            typer.Wait4SOP();
            Serial.write(code);
        }
        else if (code==ETX){
            while (!listner.isAllOff()){}
            Serial.write(code);
        }
        else if (code==ESC/*CTRL^F*/){
            while (!listner.isAllOff()){}
            detachInterrupt(scan_intr);
            typer.LoadSwitch(0x0401);
            while (digitalRead(_codePin)==LOW){}
            typer.LoadSwitch(0);
            attachInterrupt(scan_intr, onScanISR, RISING);
            Serial.write(code);
        }
        else if (code==ENQ/*CTRL^E*/){
            while (!listner.isAllOff()){}
            detachInterrupt(scan_intr);
            typer.LoadSwitch(0x0401);
            while (digitalRead(_codePin)==LOW){}
            typer.LoadSwitch(0);
            attachInterrupt(scan_intr, onScanISR, RISING);
            Serial.write(code);
        }
        else{
            typer.Wait4SOP();
            Serial.write(code);
        }    
    }
}
