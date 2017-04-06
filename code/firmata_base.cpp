/*
  Firmata is a generic protocol for communicating with microcontrollers
  from software on a host computer. It is intended to work with
  any host computer software package.

  To download a host software package, please clink on the following link
  to open the list of Firmata client libraries your default browser.

  https://github.com/firmata/arduino#firmata-client-libraries

  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2009-2015 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated by Jeff Hoefs: November 7th, 2015
*/

#include <Servo.h>
#include <Wire.h>
#include <MMA8653.h>
#include <Firmata.h>
#include <Reset.h>

#include "Arduino.h"

#if defined(ENABLE_INTERP)
#define KOOV_VERSION	"koov-1.0.11"
#else
#define KOOV_VERSION	"koov-1.0.9"
#endif

static void KoovSetup();
static void KoovLoop();

#define ON HIGH
#define OFF LOW

/*
 * 74 .. 5863 Hz.
 *
 * f = 440 * exp2((d - 69) / 12.0);
 */

const word BZR_C3  = 130;  // do
const word BZR_CS3 = 139;  // do#
const word BZR_D3  = 147;  // re
const word BZR_DS3 = 156;  // re#
const word BZR_E3  = 165;  // mi
const word BZR_F3  = 175;  // fa
const word BZR_FS3 = 185;  // fa#
const word BZR_G3  = 196;  // so
const word BZR_GS3 = 208;  // so#
const word BZR_A3  = 220;  // la
const word BZR_AS3 = 233;  // la#
const word BZR_B3  = 247;  // ti
const word BZR_C4  = 262;  // do
const word BZR_CS4 = 277;  // do#
const word BZR_D4  = 294;  // re
const word BZR_DS4 = 311;  // re#
const word BZR_E4  = 330;  // mi
const word BZR_F4  = 349;  // fa
const word BZR_FS4 = 370;  // fa#
const word BZR_G4  = 392;  // so
const word BZR_GS4 = 415;  // so#
const word BZR_A4  = 440;  // la
const word BZR_AS4 = 466;  // la#
const word BZR_B4  = 494;  // ti
const word BZR_C5  = 523;  // do
const word BZR_CS5 = 554;  // do#
const word BZR_D5  = 587;  // re
const word BZR_DS5 = 622;  // re#
const word BZR_E5  = 659;  // mi
const word BZR_F5  = 698;  // fa
const word BZR_FS5 = 740;  // fa#
const word BZR_G5  = 784;  // so
const word BZR_GS5 = 831;  // so#
const word BZR_A5  = 880;  // la
const word BZR_AS5 = 932;  // la#
const word BZR_B5  = 988;  // ti
const word BZR_C6  = 1047; // do
const word BZR_CS6 = 1109; // do#
const word BZR_D6  = 1175; // re
const word BZR_DS6 = 1245; // re#
const word BZR_E6  = 1319; // mi
const word BZR_F6  = 1397; // fa
const word BZR_FS6 = 1480; // fa#
const word BZR_G6  = 1568; // so
const word BZR_GS6 = 1661; // so#
const word BZR_A6  = 1760; // la
const word BZR_AS6 = 1865; // la#
const word BZR_B6  = 1976; // ti
const word BZR_C7  = 2093; // do
const word BZR_CS7 = 2217; // do#
const word BZR_D7  = 2349; // re
const word BZR_DS7 = 2489; // re#
const word BZR_E7  = 2637; // mi
const word BZR_F7  = 2794; // fa
const word BZR_FS7 = 2960; // fa#
const word BZR_G7  = 3136; // so
const word BZR_GS7 = 3322; // so#
const word BZR_A7  = 3520; // la
const word BZR_AS7 = 3729; // la#
const word BZR_B7  = 3951; // ti
const word BZR_C8  = 4186; // do
const word BZR_S   = 0;    // silent

const word BTONE[] = {
  BZR_C3,  BZR_CS3, BZR_D3,  BZR_DS3, BZR_E3,  BZR_F3,
  BZR_FS3, BZR_G3,  BZR_GS3, BZR_A3,  BZR_AS3, BZR_B3,
  BZR_C4,  BZR_CS4, BZR_D4,  BZR_DS4, BZR_E4,  BZR_F4,
  BZR_FS4, BZR_G4,  BZR_GS4, BZR_A4,  BZR_AS4, BZR_B4,
  BZR_C5,  BZR_CS5, BZR_D5,  BZR_DS5, BZR_E5,  BZR_F5,
  BZR_FS5, BZR_G5,  BZR_GS5, BZR_A5,  BZR_AS5, BZR_B5,
  BZR_C6,  BZR_CS6, BZR_D6,  BZR_DS6, BZR_E6,  BZR_F6,
  BZR_FS6, BZR_G6,  BZR_GS6, BZR_A6,  BZR_AS6, BZR_B6,
  BZR_C7,  BZR_CS7, BZR_D7,  BZR_DS7, BZR_E7,  BZR_F7,
  BZR_FS7, BZR_G7,  BZR_GS7, BZR_A7,  BZR_AS7, BZR_B7,
  BZR_C8,
};

#define TONENUM	((sizeof(BTONE)/sizeof(word))-1)
#define BHZ(num)  (BTONE[(byte)(min(max(0, (num-48)),TONENUM))])
static word last_pitch[14] = { 0, };
static void
BUZZER_INIT()
{

  for (int pin = 0; pin < sizeof(last_pitch) / sizeof(last_pitch[0]); pin++)
    last_pitch[pin] = 0;
}

static void
BUZZER_CONTROL(int port, int mode, int freq)
{
  if (port >= sizeof(last_pitch) / sizeof(last_pitch[0]))
    return;

  if (mode == ON) {
    word pitch = BHZ(freq);
    if (pitch != last_pitch[port]) {
      tone(port, pitch);
      last_pitch[port] = pitch;
    }
  } else {
    last_pitch[port] = 0;
    noTone(port);
  }
}

static MMA8653 accel;
static uint32_t accel_tick;

static void
ACCEL_INIT()
{

  Wire.begin(); 
  accel.begin(false, 2);
}

static void
ACCEL_UPDATE(int *x, int *y, int *z)
{
  uint32_t tick = millis();

  if (tick - accel_tick > 2000) {
    ACCEL_INIT();
    accel_tick = tick;
  }
  accel.update();
  *x = accel.getX();
  *y = accel.getY();
  *z = accel.getZ();

#define ACCEL_MAP(n)  (map((n), -128, 127, -100, 100))
  *x = ACCEL_MAP(*x);
  *y = ACCEL_MAP(*y);
  *z = ACCEL_MAP(*z);
#undef ACCEL_MAP
}

struct servo_sync {
  byte port;
  byte degree;
};
#define servo_sync servo_sync

static void
SERVOMOTOR_SYNCHRONIZED_MOTION(struct servo_sync *ss, int number, byte time);
static void
MULTILED(int r, int g, int b);

static uint8_t interp_error = 0;
