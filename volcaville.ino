// Arduino-based MIDI interface, customised for VOLCA sample & fm
//   -- it's mostly a USB -> MIDI 5-pin OUT interface now
//
// MIDI data pin is 1(TX)
// expects volca fm on MIDI channel 11

// MIDIUSB spec: https://www.usb.org/sites/default/files/midi10.pdf

#include <MIDIUSB.h>

byte lit=0;

static constexpr byte led_pin = 13;
static constexpr byte mode_pin = 2;


void toggleLed() {
  lit = 1 - lit;
  digitalWrite(led_pin,lit);
}

void setup() {
  pinMode(led_pin,OUTPUT);

  // the mode switch is across GND and pin3, so we set 3 HIGH forever
  pinMode(3,OUTPUT);
  digitalWrite(3, HIGH);
  
  pinMode(mode_pin, INPUT);
  
  Serial1.begin(31250);
}

byte oldVolcaMode = 0;
void loop() {

  byte volcaMode = digitalRead(mode_pin);
  if (volcaMode != oldVolcaMode) {
    lit = volcaMode;
    digitalWrite(led_pin, lit);
    oldVolcaMode = volcaMode;
  }
 
  midiEventPacket_t message = MidiUSB.read();

  // ignore virtual cable number for now - we're just one
  int cin = message.header & 0xf;

  int chan = message.byte1 & 0xf;

  switch (cin)
  {
    case 0:   // reserved
      break;
    case 1:   // cable event; reserved
      break;

    case 2:   // 2 byte system-common
      midiSend2(message.byte1, message.byte2);
      break;

    case 3:   // 3 byte system common
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 4:   // 3 byte SysEx (start / continue)
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 5:   // 1 byte system common / final SysEx byte
      midiSend1(message.byte1);
      break;

    case 6:   // final 2 bytes of SysEx
      midiSend2(message.byte1, message.byte2);
      break;
      
    case 7:   // final 3 bytes of SysEx
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 8:   // note off
      // Volca SAMPLE only cares about noteOn messages
      if (!volcaMode || chan >= 10)
      {
        toggleLed();
        midiSend3(message.byte1, message.byte2, message.byte3);
      }
      break;

    case 9:   // note on
      toggleLed();
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 0xa: // aftertouch
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 0xb: // cc
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 0xc: // program change
      midiSend2(message.byte1, message.byte2);
      break;

    case 0xd: // channel pressure
      midiSend2(message.byte1, message.byte2);
      break;

    case 0xe: // pitch bend
      midiSend3(message.byte1, message.byte2, message.byte3);
      break;

    case 0xf: // single-byte, e.g. realtime (Start, Stop, Clock, ...)
      if (!volcaMode)
      {
        midiSend1(message.byte1);
        
        /* -- using MIDI to sync directly has a lot of downsides, esp. for Volcas
         *  --> would be better to add a 3.5mm SYNC OUT socket where we send sync to VOLCAs & POs
        if( message.byte1 == 0xf8 || (message.byte1 >= 0xfa && message.byte1 <= 0xfc)  )
        {
          midiSend1(message.byte1);
        }
        */
      }
      break;
  }
}


void midiSend1(int b0) {
  Serial1.write(b0);
}
void midiSend2(int b0, int b1) {
  Serial1.write(b0);
  Serial1.write(b1);
}
void midiSend3(int b0, int b1, int b2) {
  Serial1.write(b0);
  Serial1.write(b1);
  Serial1.write(b2);
}
