// Arduino-based MIDI interface, customised for VOLCA sample & fm
// MIDI data pin is 1(TX)
// expects volca fm on MIDI channel 11

// MIDIUSB spec: https://www.usb.org/sites/default/files/midi10.pdf

#include <MIDIUSB.h>

int lit=0;


void setup() {
  pinMode(13,OUTPUT);
  Serial1.begin(31250);
}

void loop() {
 
  midiEventPacket_t message = MidiUSB.read();

  // ignore virtul cable number for now - we're just one
  int cin = message.header & 0xf;

  int chan = message.byte1 & 0xf;
  // SAMPLE only cares about noteOn messages
  if( (cin == 8 && chan >= 10) || cin == 9)
  {
    lit = 1-lit;
    digitalWrite(13,lit);
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
