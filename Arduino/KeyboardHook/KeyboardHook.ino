#include "SpiBus.h"
#include "Listner.h"
#include "Printer.h"
#include <MsTimer2.h>
#include "AsciiCode.h"

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

uint8_t scan_intr;

Printer printer(scan_intr, _codePin, outputLatchPin, _printingPin, feedingPin, Waiting4SOP, Waiting4SOF, Waiting4EOA);
Listner listner(&printer, inputLatchPin, _capsPin, _codePin);

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
    //Serial.print("interrupt:");Serial.println(scan_intr);
    SpiBus::begin();
    listner.begin();
    printer.begin();
#ifdef USE_CTS    
    digitalWrite(_ctsPin,LOW);
#endif     
    attachInterrupt(scan_intr, onScanISR, RISING);
}

void onScanISR() {
    listner.onScan();
}

void loop() {
    if (Serial.available() > 0) {
        if ((digitalRead(_codePin)==HIGH)&&listner.isAllOff()){
            printer.type(Serial.read());
            Serial.write(ASC_ACK);    
        }
    }
    int code = listner.getAsciiCode();
    if (code>ASC_NUL){
        Serial.write(code);
    }
}
