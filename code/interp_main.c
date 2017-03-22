static const uint8_t scripts[] __attribute__((section("KOOV_DATA"),
					      aligned(256))) = {
  0x00, 0x00
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
}

void
loop()
{
#if 0
  SerialUSB.println("exec!");
#endif
  int err = interp_exec(scripts, 0);

#if 0
  if (err) {
    while (digitalRead(BUTTON_A0) == HIGH) {
      SerialUSB.println(err);
      delay(10);
    }
  }
#endif
}
