/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

static const uint8_t scripts[] __attribute__((section("KOOV_DATA"),
                                              aligned(256))) = {
#if !defined(INCLUDE_TRANSLATED_C)
  0x26, 0x00, 0x03, 0x20, 0x00, 0x02, 0x00, 0x04,
  0x21, 0x00, 0x1c, 0x00, 0x03, 0x19, 0x00, 0x02,
  0x0d, 0x00, 0x01, 0x00, 0x04, 0x1a, 0x00, 0x0f,
  0x00, 0x03, 0x0c, 0x00, 0x02, 0x0d, 0x00, 0x03,
  0x00, 0x04, 0x1a, 0x00, 0x02, 0x00,
#else
#include "translated.c"
#endif

  KOOV_MAGIC & 0xff,
  (KOOV_MAGIC >> 8) & 0xff,
  (KOOV_MAGIC >> 16) & 0xff,
  (KOOV_MAGIC >> 24) & 0xff,
};

void setup()
{
#if 0
#define BUTTON_A0 24
#define BUTTON_A1 25
#define BUTTON_A2 26
#define BUTTON_A3 27
  pinMode(BUTTON_A0,INPUT_PULLUP);
  pinMode(BUTTON_A1,INPUT_PULLUP);
  pinMode(BUTTON_A2,INPUT_PULLUP);
  pinMode(BUTTON_A3,INPUT_PULLUP);
  SerialUSB.begin(38400);
#endif
  firmata_base::systemResetCallback();  // reset to default config
}

static int
post_exec()
{
  INIT_OUTPUTS();
  int err = interp_setup(scripts, 0);
  if (err)
    return err;
  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
  }
}

void
loop()
{
#if 0
  SerialUSB.println("exec!");
#endif
  if (!interp_error) {
    randomSeed(analogRead(0));
    INIT_OUTPUTS();
    int err = interp_exec(scripts, 0);

    if (!err)
      err = post_exec();

    if (err && err != ERROR_INTERRUPTED) {
      interp_error = err;
#if 0
      while (digitalRead(BUTTON_A0) == HIGH) {
        SerialUSB.println(err);
        delay(10);
      }
#endif
    }
  }
}
