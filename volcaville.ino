// Arduino-based MIDI interface, customised for VOLCA sample & fm
// MIDI data pin is 1(TX)
// expects volca fm on MIDI channel 11

// MIDIUSB spec: https://www.usb.org/sites/default/files/midi10.pdf

#include <MIDIUSB.h>

byte lit=0;

static const byte led_pin = 13;
static const byte mode_pin = 2;


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
  // SAMPLE only cares about noteOn messages
  if( (cin == 8 && (!volcaMode || chan >= 10)) || cin == 9)
  {
    lit = 1-lit;
    digitalWrite(led_pin,lit);
    midiSend3(message.byte1, message.byte2, message.byte3);
  }

  // is it 3 bytes of sysex? (either start, cont, or end)
  else if( cin == 4 || cin == 7)
  {
    midiSend3(message.byte1, message.byte2, message.byte3);
  }
  else if( cin == 6 )
  {
    midiSend2(message.byte1, message.byte2);
  }
  else if( cin == 5 )
  {
    midiSend1(message.byte1);
  }
  else if( cin == 0xb )
  {
    midiSend3(message.byte1, message.byte2, message.byte3);
  }
  else if( cin == 0xc )
  {
    midiSend2(message.byte1, message.byte2);
  }
  else if( cin == 0xe )
  {
    midiSend3(message.byte1, message.byte2, message.byte3);
  }
  else if( cin == 0xd )
  {
    midiSend2(message.byte1, message.byte2);
  }

  /* -- using MIDI to sync directly has a lot of downsides
   *  --> would be better to add a 3.5mm SYNC OUT socket where we send sync to VOLCAs & POs
  else if( message.byte1 == 0xf8 || (message.byte1 >= 0xfa && message.byte1 <= 0xfc)  )
  {
    midiSend1(message.byte1);
  }
  */
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
