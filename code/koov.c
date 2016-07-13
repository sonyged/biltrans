#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif
#define clamp(MIN, MAX, VALUE)	(max((MIN), min((MAX), (VALUE))))

#define setup KoovSetup
#define loop KoovLoop

/* port# to board pin# mapping */
#define PORT_V2 2
#define PORT_V3 3
#define PORT_V4 6
#define PORT_V5 7
#define PORT_V6 8
#define PORT_V7 9
#define PORT_V8 11
#define PORT_V9 13

#define PORT_K2 24
#define PORT_K3 25
#define PORT_K4 26
#define PORT_K5 27
#define PORT_K6 28
#define PORT_K7 29

#define LED_MULTI_FET 18
#define LED_MULTI_RED 13
#define LED_MULTI_GREEN 12
#define LED_MULTI_BLUE 10

#define PORT_A0 PORT_K2
#define PORT_A1 PORT_K3
#define PORT_A2 PORT_K4
#define PORT_A3 PORT_K5

#define ON HIGH
#define OFF LOW
#define TURN_LED(P, M)  digitalWrite((P), (M))
static void delay_with_chech(unsigned long msec)
{
  while (msec > 0) {
    unsigned long t = msec < 10 ? msec : 10;
    delay(t);
    CHECK_INTR;
    msec -= t;
  }
}
#define DELAY(S)	delay_with_chech((S) * 1000UL)

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
static void
BUZZER_CONTROL(int port, int mode, int freq)
{
  if (mode == ON) {
    word pitch = BHZ(freq);
    tone(port, pitch);
  } else {
    noTone(port);
  }
}

enum { PORT_V0 = 0, PORT_V1 };

enum dcMotroMode {
  DCMOTOR_NORMAL,
  DCMOTOR_REVERSE,
  DCMOTOR_COAST,
  DCMOTOR_BRAKE
};

static struct {
  int aport;
  int dport;
  int power;
  enum dcMotroMode mode;
} dcMotorState[2] = {
  { 4, 5, 0, DCMOTOR_COAST },
  { 12, 10, 0, DCMOTOR_COAST }
};

static void
INIT_DC_MOTOR(int port)
{

  switch (port) {
  default:
    return;
  case PORT_V0:
  case PORT_V1:
    pinMode(dcMotorState[port].dport, OUTPUT);
    pinMode(dcMotorState[port].aport, OUTPUT);
    break;
  }
  pinMode(LED_MULTI_FET, OUTPUT);
  digitalWrite(LED_MULTI_FET, HIGH);
}

static const int analogMax = 255;
static void
DCMOTOR_CONTROL(int port)
{
  int dport = dcMotorState[port].dport;
  int aport = dcMotorState[port].aport;
  int power = dcMotorState[port].power;

  switch (dcMotorState[port].mode) {
  case DCMOTOR_NORMAL:
    digitalWrite(dport, LOW);
    analogWrite(aport, power);
    break;
  case DCMOTOR_REVERSE:
    digitalWrite(dport, HIGH);
    analogWrite(aport, analogMax - power);
    break;
  case DCMOTOR_COAST:
    digitalWrite(dport, LOW);
    analogWrite(aport, 0);
    break;
  case DCMOTOR_BRAKE:
    digitalWrite(dport, HIGH);
    analogWrite(aport, analogMax);
    break;
  }
}

static void
SET_DCMOTOR_POWER(int port, int power)
{

  power = clamp(0, 100, power);
  power = map(power, 0, 100, 0, analogMax);

  switch (port) {
  default:
    return;
  case PORT_V0:
  case PORT_V1:
    dcMotorState[port].power = power;
    break;
  }
  DCMOTOR_CONTROL(port);
}

static void
SET_DCMOTOR_MODE(int port, dcMotroMode mode)
{

  switch (mode) {
  default:
    return;
  case DCMOTOR_NORMAL:
  case DCMOTOR_REVERSE:
  case DCMOTOR_COAST:
  case DCMOTOR_BRAKE:
    break;
  }

  switch (port) {
  default:
    return;
  case PORT_V0:
  case PORT_V1:
    dcMotorState[port].mode = mode;
    break;
  }
  DCMOTOR_CONTROL(port);
}

static void
SERVO_MOTOR(int port, int value)
{
  int pin = port + 8;
  if (servoPinMap[pin] < MAX_SERVOS)
    servos[servoPinMap[pin]].write(value);
}
static void
INIT_SERVO_MOTOR(int port)
{
  int pin = port + 8;
  attachServo(pin, 0, 0);
  pinMode(LED_MULTI_FET, OUTPUT);
  digitalWrite(LED_MULTI_FET, HIGH);
}

struct servo_sync {
  int port;
  int degree;
};

static void
SERVOMOTOR_SYNCHRONIZED_MOTION(struct servo_sync *ss, int number, byte time)
{
  byte before[8];        // Current angles of servomotor
  double delta[8];       // delta <- target angle - current angle
  byte maxDelta = 0;     // max of delta
  byte calibedDegree;    // Temporary value for calibrating angle.

  // If there are not servomotors setting angle, do nothing.
  if (number == 0) return;

  time = min(max(0, (20 - time)), 20);

  // Get maximum difference between current angle and target angle.
  for (int i = 0;i < number;i++) {
    int pin = ss[i].port + 8;
    int degree = ss[i].degree;
    before[i] = servos[servoPinMap[pin]].read();  // Current angle.
    calibedDegree = min(180, max(0, degree /* + SVOFF[connector[i]] */)); // Calibrating the given angle.
    delta[i] = calibedDegree - before[i];              // Get difference.
    // Get maximum difference.
    maxDelta = (abs(delta[i]) > maxDelta) ? abs(delta[i]) : maxDelta;
  }

  // Set angles for each servomotor
  if (time == 0) {  // If delay time is 0...
    // Set angles for each servomotor
    for (int i = 0;i < number;i++) {
      int pin = ss[i].port + 8;
      int degree = ss[i].degree;
      calibedDegree = min(180, max(0, degree /* + SVOFF[connector[i]] */)); // Calibrating the given angle.
      servos[servoPinMap[pin]].write(calibedDegree);
    }
    // Wait until all servomotors reach their target angles.
    delay(maxDelta * 3);
  } else {          // If delay time is over 1...
    for (int i = 0;i < number;i++) {
      delta[i] = (double)(delta[i]) / (double)(maxDelta);
    }
    // Set angles for each servomotor
    for (int t = 1; t <= (int)maxDelta; t++) {
      for (int i = 0; i < number; i++) {
	int pin = ss[i].port + 8;
        servos[servoPinMap[pin]].write(before[i]+delta[i]*t);
      }
      delay(time);
    }
  }
}

static int
sensor_value(int port)
{

  return map(analogRead(port), 0, 1023, 0, 100);
}

static void
init_sensor(int port)
{

  pinMode(port, INPUT);
  analogReference(AR_DEFAULT);
}

#define LIGHT_SENSOR(port)    (sensor_value(port))
#define INIT_LIGHT_SENSOR(port) (init_sensor(port))
#define IR_PHOTO_REFLECTOR(port)    (sensor_value(port))
#define INIT_IR_PHOTO_REFLECTOR(port) (init_sensor(port))

#define TOUCH_SENSOR(port)    (digitalRead(port))
static void
INIT_TOUCH_SENSOR(int port)
{

  pinMode(port, INPUT);
}

static int
PUSH_BUTTON(int port)
{

  return digitalRead(port) == LOW ? 0 : 1;
}

static void
INIT_PUSH_BUTTON(int port)
{

  pinMode(port, INPUT_PULLUP);
}

static void
INIT_MULTILED()
{
  pinMode(LED_MULTI_RED, OUTPUT);
  pinMode(LED_MULTI_GREEN, OUTPUT);
  pinMode(LED_MULTI_BLUE, OUTPUT);
  pinMode(LED_MULTI_FET, OUTPUT);

  digitalWrite(LED_MULTI_RED, HIGH);
  digitalWrite(LED_MULTI_GREEN, HIGH);
  digitalWrite(LED_MULTI_BLUE, HIGH);
  digitalWrite(LED_MULTI_FET, HIGH);
}

static void
MULTILED(int r, int g, int b)
{
  static const int analogMax = 255;

  r = map(clamp(0, 100, r), 0, 100, 0, analogMax);
  g = map(clamp(0, 100, g), 0, 100, 0, analogMax);
  b = map(clamp(0, 100, b), 0, 100, 0, analogMax);

  if (r == 0 && g == 0 && b == 0) {
    digitalWrite(LED_MULTI_RED, HIGH);
    digitalWrite(LED_MULTI_GREEN, HIGH);
    digitalWrite(LED_MULTI_BLUE, HIGH);
    digitalWrite(LED_MULTI_FET, HIGH);
    return;
  }

  digitalWrite(LED_MULTI_FET, LOW);
  analogWrite(LED_MULTI_RED, analogMax - r);
  analogWrite(LED_MULTI_GREEN, analogMax - g);
  analogWrite(LED_MULTI_BLUE, analogMax - b);
}
