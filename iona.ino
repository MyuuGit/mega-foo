//mega-foo by MyuuGit
//this is a cutting edge branch of iona-mega, for experimental purposes only, apply topically.
//This software based on iona-mega which is Copyright 2018 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
//
//BEGIN PROGRAM

//Load the Client file, this tells the program how to map the pins.
#include "jvsio/clients/MegaClient.h"

//and the jvs io protocol by toyoshim
#include "jvsio/JVSIO.cpp"

//setting up comms and human interfaces here:
MegaDataClient data;
MegaSenseClient sense;
MegaLedClient led;
JVSIO io(&data, &sense, &led);

// Mega wil will support 6buttons per player mode, and analog by default. add more below.
// Some NAOMI games expect the first segment starts with "SEGA ENTERPRISES,LTD.".
// E.g. one major official I/O board is "SEGA ENTERPRISES,LTD.;I/O 838-13683B;Ver1.07;99/16".
// Here we use a custom one, because its cool and hasn't broken anything yet.
static const char io_id[] = "SEGA ENTERPRISES,LTD.;I/O 838-13683B;Ver1.07;99/16";
static const char suchipai_id[] = "SEGA ENTERPRISES,LTD.compat;IONA-Mega;Ver1.01;Su Chi Pai Mode";
static const char virtualon_id[] = "SEGA ENTERPRISES,LTD.compat;IONA-Mega;Ver1.01;Virtual-On Mode";
//IO Test, IO Player 1 UDLRABSQ, IO Player CXYZLR, IO Player 2 UDLRABSQ, IO Player 2 CXYZLR
uint8_t ios[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t coinCount = 0;
uint8_t mode = 0;
uint8_t coin = 0;
uint8_t gpout = 0;
//setup the amount of ios, clear the coin count, set the default mode (could set it to permanant mahjongg mode or virtual on on boot) and the amount of gpout, perhaps some games have a win counter or etc.
bool suchipai_mode = false;
bool virtualon_mode = false;
int in(int pin, int shift) {
  return (digitalRead(pin) ? 0 : 1) << shift;
}
void updateMode() {
int value = 0;
if (value < 170)//here we nope it to ghost switches, set them manually
    mode = 0;//here we nope it to ghost switches, set them manually
  else if (value < 1)//here we nope it to ghost switches, set them manually
    mode = 1; //here we nope it to ghost switches, set them manually
  else if (value < 2)//here we nope it to ghost switches, set them manually
    mode = 2;//here we nope it to ghost switches, set them manually
  else //here we nope it to ghost switches, set them manually
    mode = 3;//here we nope it to ghost switches, set them manually
  suchipai_mode = mode == 2;//here we nope it to ghost switches, set them manually
  virtualon_mode = mode == 1;//here we nope it to ghost switches, set them manually
}
//end alternate mode setup
uint8_t suchipaiReport() {
  // OUT  |  B7 |  B6 |  B5 |  B4 |  B3 |  B2 |  B1 |  B0 |
  // -----+-----+-----+-----+-----+-----+-----+-----+-----+
  // 0x40 |  A  |  -  |  E  |  I  |  M  | KAN |  -  |  -  |
  // 0x20 |  B  |  -  |  F  |  J  |  N  |REACH|START|  -  |
  // 0x10 |  C  |  -  |  G  |  K  | CHI | RON |  -  |  -  |
  // 0x80 |  D  |  -  |  H  |  L  | PON |  -  |  -  |  -  |

  // Emulated by D-pad + 4 buttons.
  uint8_t start = (ios[1] & 0x80) ? 2 : 0;
  if ((gpout == 0x40 && !(ios[1] & 0x02)) ||
      (gpout == 0x20 && !(ios[1] & 0x01)) ||
      (gpout == 0x10 && !(ios[2] & 0x80)) ||
      (gpout == 0x80 && !(ios[2] & 0x40))) {
    return start;
  }
  switch (ios[1] & 0x2c) {
   case 0x00:
    return 0x00 | start;
   case 0x04:
    return 0x04 | start;
   case 0x08:
    return 0x80 | start;
   case 0x20:
    return 0x10 | start;
   case 0x24:
    return 0x08 | start;
   case 0x28:
    return 0x20 | start;
  }
  return start;
}
//switch yadda to make the mahjong go, it looks like its setting bits manually incrementally.
uint8_t virtualonReport(size_t player, size_t line) {
  //       |  B7 |  B6 |  B5 |  B4 |  B3 |  B2 |  B1 |  B0 |
  // ------+-----+-----+-----+-----+-----+-----+-----+-----+
  // P0-L1 |Start|  -  | L U | L D | L L | L R |L sht|L trb|
  // P0-L2 | QM  |  -  |  -  |  -  |  -  |  -  |  -  |  -  |
  // P1-L1 |  -  |  -  | R U | R D | R L | R R |R sht|R trb|
  // P1-L2 |  -  |  -  |  -  |  -  |  -  |  -  |  -  |  -  |

  // Try fullfilling 2 stick controlls avobe via usual single
  // arcade controller that has only 1 stick and 4 buttons.
  // Button 1 - L shot
  // Button 2 - turn or jump w/ stick
  // Button 3 - LR turbo
  // Button 4 - R shot
  
  if (player >= 2 || line != 1)
    return 0x00;
  uint8_t data = 0x00;
  bool rotate = ios[1] & 1;
  if (rotate) {
    bool up = ios[1] & 32;
    bool down = ios[1] & 16;
    bool left = ios[1] & 8;
    bool right = ios[1] & 4;
    if (player == 0) { // Left Stick
      if (left) data |= 32;
      if (right) data |= 16;
      if (up) data |= 8;
      if (down) data |= 4;
    } else {  // Right Stick
      if (left) data |= 16;
      if (right) data |= 32;
      if (up) data |= 4;
      if (down) data |= 8;
    }
  } else {
    data = ios[1] & ~3;
  }
  if (player == 0 && ios[1] & 2)
    data |= 2;
  if (player == 1 && ios[2] & 64)
    data |= 2;
  if (ios[2] & 128)
    data |= 1;
  return data;
}
//Setup The analog pin array
void setup() {
  int analog[] = {A0,A1,A2,A3};
  for (int q = 0; q < sizeof(analog); ++q)
    pinMode(analog[q], INPUT);
// IO BEGIN
  io.begin();
  //Digital Pins for Joy and Buttons
  //set up the digital pin array
  int pins[] = {4,5,6,7,8,9,14,18,19,20,21,22,52,31,32,33,35,37,39,41,43,45,47,49,51,99};
  for (int i = 0; i < sizeof(pins); ++i)
    pinMode(pins[i], INPUT_PULLUP);
    //specificially declare the coin to be pullup to remove glitch
    pinMode(53, INPUT_PULLUP);
    //supposedly set up the analog
}
//Setup Potenteometer Zeroes and ANALOGREADs.
void loop() {
  uint8_t len;
  uint8_t* data = io.getNextCommand(&len);
  uint16_t analog[8] = {
    0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000
  };

//ANALOG TO DIGITAL MAP EXPERIMENT
int Us = !digitalRead(21);
int Ds = !digitalRead(20);
int Ls = !digitalRead(19);
int Rs = !digitalRead(18);
int As = !digitalRead(4);
int Bs = !digitalRead(5);
//int Cs = !digitalRead(6);
//int Xs = !digitalRead(7);
//int Ys = !digitalRead(8);
//int Zs = !digitalRead(9);
if (Us == HIGH) {if (Ds == LOW)  {analog[0] = {0x0000};} else {analog[0] = {0x8000};}
}
if (Ds == HIGH) {if (Ls == LOW)  {analog[0] = {0xFF00};} else {analog[0] = {0x8000};}
}
if (Ls == HIGH) {if (Rs == LOW)  {analog[1] = {0x0000};} else {analog[1] = {0x8000};}
}
if (Rs == HIGH) {if (Ls == LOW)  {analog[1] = {0xFF00};} else {analog[1] = {0x8000};}
}
if (As == HIGH) {if (As == HIGH)  {analog[2] = {0xFF00};} else {analog[3] = {0x8000};}
}
if (Bs == HIGH) {if (Bs == HIGH)  {analog[3] = {0xFF00};} else {analog[3] = {0x8000};}
}
//if (Cs == HIGH) {if (Cs == HIGH)  {analog[4] = {0xFF00};} else {analog[4] = {0x8000};}
//}
//if (Xs == HIGH) {if (Xs == HIGH)  {analog[5] = {0xFF00};} else {analog[5] = {0x8000};}
//}
//if (Ys == HIGH) {if (Ys == HIGH)  {analog[6] = {0xFF00};} else {analog[3] = {0x8000};}
//}
//if (Zs == HIGH) {if (Zs == HIGH)  {analog[7] = {0xFF00};} else {analog[4] = {0x8000};}
//}

//ASSIGN TRUE ANALOGS WITH THIS:
//analog[0] = analogRead(A0) << 6;
//analog[1] = analogRead(A1) << 6;
//analog[2] = analogRead(A2) << 6;
//analog[3] = analogRead(A3) << 6;
//analog[4] = analogRead(A4) << 6;
//analog[5] = analogRead(A5) << 6;
//analog[6] = analogRead(A6) << 6;
//analog[7] = analogRead(A7) << 6;


  if (!data) {
    updateMode();

    // PIN TO BIT MAPPING
//These are all messed up because I'm testing things. please set them accordingly.
    //IT GOES LIKE SO, BIT 7 to 0, IOS 0 TO 4,
    //TEST BUTTON IO 0 BIT 7
    ios[0] = in(32, 7);
        //Start Button IO 1 Bit 7 | Service Button IO 1 Bit 6 | Up Button Bit IO 1 5 | DnButton Bit IO 1 4 |Lft Button IO 1 Bit 3 |Rgt Button IO 1 Bit 2 |A Button IO 1 Bit 1 |B Button IO 1 Bit 0;
    ios[1] = in(14,7) | in(32,6)| in(21,5) | in(20,4) | in(19,3) | in(18,2) | in(4,1)| in(5,0);
    
         //  C Button IO 2 Bit 7|X Button IO 2 Bit 6 |Y Button IO 2 Bit 5 | Z Button IO 2 Bit 4 
    ios[2] = in(6,7) | in(7,6) | in(8,5) | in(9,4);
    
//ditto 2p
    //Start Button IO 3 Bit 7 | Service Button IO 3 Bit 6 | Up Button Bit IO 3 5 | DnButton Bit IO 3 4 |Lft Button IO 3 Bit 3 |Rgt Button IO 3 Bit 2 |A Button IO 3 Bit 1 |B Button IO 3 Bit 0;
    ios[3] = in(31, 7) | in( 33, 5)| in( 35, 4) | in( 37, 3) | in( 39, 2) | in( 41, 1) | in( 43, 0);
//  C Button IO 4 Bit 7|X Button IO 4 Bit 6 |Y Button IO 4 Bit 5 | Z Button IO 4 Bit 4 
    ios[4] = in(45, 7) | in(47, 6) | in(49, 5) | in(51, 4);
//CREDIT SLOT
 //   Remember to set it to input pullup by hand upstairs or it will go squirly, otherwise maybe you want it to chuck in many coins at once randomly?
    uint8_t newCoin = digitalRead(53);
    if (coin && !newCoin)
      coinCount++;
    coin = newCoin;
    return;
  }
  switch (*data) {
   case JVSIO::kCmdReset:
    coinCount = 0;
    break;
   case JVSIO::kCmdIoId:
    io.pushReport(JVSIO::kReportOk);
    {
      const char* id = virtualon_mode ? virtualon_id : suchipai_mode ? suchipai_id : io_id;
      for (size_t i = 0; id[i]; ++i)
        io.pushReport(id[i]);
    }
    io.pushReport(0);
    break;
   case JVSIO::kCmdFunctionCheck:
    io.pushReport(JVSIO::kReportOk);
//Switches
    io.pushReport(0x01);  // sw
    io.pushReport(0x02);  // players
    io.pushReport(0x0C);  // buttons
    io.pushReport(0x00);
//Coins
    io.pushReport(0x02);  // coin
    io.pushReport(0x02);  // slots
    io.pushReport(0x00);
    io.pushReport(0x00);
//Analog
    io.pushReport(0x03);  // analog inputs
    io.pushReport(0x08);  // channels
    io.pushReport(0x00);  // bits
    io.pushReport(0x00);
    //GPIO (Winner lights, led counter, etc)
    io.pushReport(0x12);  // general purpose driver
    io.pushReport(0x08);  // slots
 //Mystery   
    io.pushReport(0x00);
    io.pushReport(0x00);
    io.pushReport(0x00);
    break;
   case JVSIO::kCmdSwInput:
    io.pushReport(JVSIO::kReportOk);
    io.pushReport(ios[0]);
    for (size_t player = 0; player < data[1]; ++player) {
      for (size_t line = 1; line <= data[2]; ++line) {
        if (virtualon_mode) {
          io.pushReport(virtualonReport(player, line));
        } else if (suchipai_mode) {
          io.pushReport(suchipaiReport());
        } else {
          int index = player * 2 + line;
          io.pushReport(index < 5 ? ios[index] : 0x00);
        }
      }
    }
    break;
   case JVSIO::kCmdCoinInput:
    io.pushReport(JVSIO::kReportOk);
    for (size_t slot = 0; slot < data[1]; ++slot) {
      io.pushReport((0 << 6) | 0);
      io.pushReport(slot ? 0x00 : coinCount);
    }
    break;
   case JVSIO::kCmdAnalogInput:
    io.pushReport(JVSIO::kReportOk);
    for (size_t channel = 0; channel < data[1]; ++channel) {
      io.pushReport(analog[channel] >> 8);
      io.pushReport(analog[channel] & 0xff);
    }
    break;
   case JVSIO::kCmdCoinSub:
    coinCount -= data[3];
    io.pushReport(JVSIO::kReportOk);
    break;
   case JVSIO::kCmdCoinAdd:
    coinCount += data[3];
    io.pushReport(JVSIO::kReportOk);
    break;
   case JVSIO::kCmdDriverOutput:
    gpout = data[2];
    io.pushReport(JVSIO::kReportOk);
    break;
  }
}
