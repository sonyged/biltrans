/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif
#define clamp(MIN, MAX, VALUE)	(max((MIN), min((MAX), (VALUE))))

static void (*current_loop)() = firmata_base::loop;
static void DCMOTOR_CONTROL(int port);

void
setup()
{

  pinMode(A1, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);

  if (digitalRead(A1) == LOW && digitalRead(A3) == LOW) {
    trouble_shooting::setup();
    current_loop = trouble_shooting::loop;
  } else {
    firmata_base::setup();
  }
}

void
loop()
{

  (*current_loop)();
}

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

enum accel_port { PORT_K0 = 0, PORT_K1 };

void
INIT_3_AXIS_DIGITAL_ACCELEROMETER(accel_port port)
{

  // currently, only single accelerometer is supported.
  ACCEL_INIT();
}

static int
ACCELEROMETER_VALUE(int port, int direction)
{
  int x, y, z, v = 0;

  ACCEL_UPDATE(&x, &y, &z);
  switch (direction) {
  case 1: v = x; break;
  case 2: v = y; break;
  case 3: v = z; break;
  }
  return v;
}

enum { PORT_V0 = 0, PORT_V1 };

enum dcMotroMode {
  DCMOTOR_NORMAL,
  DCMOTOR_REVERSE,
  DCMOTOR_COAST,
  DCMOTOR_BRAKE
};

const int DCMOTOR_INITIAL_POWER = 30;

static struct {
  int aport;
  int dport;
  int power;
  enum dcMotroMode mode;
} dcMotorState[2] = {
  { 4, 5, DCMOTOR_INITIAL_POWER, DCMOTOR_COAST },
  { 12, 10, DCMOTOR_INITIAL_POWER, DCMOTOR_COAST }
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
    dcMotorState[port].mode = DCMOTOR_COAST;
    dcMotorState[port].power = clamp(0, 100, DCMOTOR_INITIAL_POWER);
    break;
  }
  pinMode(LED_MULTI_FET, OUTPUT);
  digitalWrite(LED_MULTI_FET, HIGH);
  DCMOTOR_CONTROL(port);
}

#ifndef ARRAYCOUNT
#define ARRAYCOUNT(a)	((sizeof (a)) / sizeof((a)[0]))
#endif
#define INTERPOLATE(x, minx, maxx, miny, maxy)			\
  ((((maxy) - (miny)) * (clamp((minx), (maxx), (x)) - (minx)) /	\
    ((double)((maxx) - (minx)))) + (miny))

static struct rpm_table {
  int power;
  double rpm;
} normal_rpm_table[] = {
  { 0, 0 },
  { 10, 0 },
  { 20, 0 },
  { 30, 22.5 },
  { 40, 35.4 },
  { 50, 45.7 },
  { 60, 58.2 },
  { 70, 65.8 },
  { 80, 68.2 },
  { 90, 69.7 },
  { 100, 71.2 },
}, reverse_rpm_table[] = {
  { 0, 0 },
  { 10, 0 },
  { 20, 3.8 },
  { 30, 8.3 },
  { 40, 15.2 },
  { 50, 19.2 },
  { 60, 29.9 },
  { 70, 48.6 },
  { 80, 58.5 },
  { 90, 65.5 },
  { 100, 71.3 },
};

#define DCMOTOR_RPM_MAX 70
#define DCMOTOR_RPM_MIN 25

#define DCMOTOR_POWER_SWITCH 10

#if 0
static double
dcmotor_rpm(double power, bool normal)
{
  struct rpm_table *table = normal ? normal_rpm_table : reverse_rpm_table;
  const int count =
    normal ? ARRAYCOUNT(normal_rpm_table) : ARRAYCOUNT (reverse_rpm_table);

  for (int i = 1; i < count; i++) {
    struct rpm_table *cur = &table[i];
    struct rpm_table *prev = &table[i - 1];
    if (prev->power <= power && power <= cur->power) {
      return INTERPOLATE(power, prev->power, cur->power, prev->rpm, cur->rpm);
    }
  }
  return 0;
}
#endif

/*
 * Correct dcmotor power for each direction.
 */
static double
dcmotor_correct(double power, bool normal)
{
  struct rpm_table *table = normal ? normal_rpm_table : reverse_rpm_table;
  const int count =
    normal ? ARRAYCOUNT(normal_rpm_table) : ARRAYCOUNT (reverse_rpm_table);

  if (power < DCMOTOR_POWER_SWITCH) {
    double power_switch = dcmotor_correct(DCMOTOR_POWER_SWITCH, normal);
    return power_switch * power / DCMOTOR_POWER_SWITCH;
  }

  double rpm = INTERPOLATE(power, DCMOTOR_POWER_SWITCH, 100,
			   DCMOTOR_RPM_MIN, DCMOTOR_RPM_MAX);

  for (int i = 1; i < count; i++) {
    struct rpm_table *cur = &table[i];
    struct rpm_table *prev = &table[i - 1];

    if (prev->rpm <= rpm && rpm <= cur->rpm) {
      return INTERPOLATE(rpm, prev->rpm, cur->rpm, prev->power, cur->power);
    }
  }
  return 0;
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
    if (power > 0) {
      double dpower = dcmotor_correct(power, true);
      power = INTERPOLATE(dpower, 0, 100, 0, analogMax);
    }
    analogWrite(aport, power);
    break;
  case DCMOTOR_REVERSE:
    digitalWrite(dport, HIGH);
    if (power > 0) {
      double dpower = dcmotor_correct(power, false);
      power = INTERPOLATE(dpower, 0, 100, 0, analogMax);
    }
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

  switch (port) {
  default:
    return;
  case PORT_V0:
  case PORT_V1:
    dcMotorState[port].power = clamp(0, 100, power);
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
  int servoNo = firmata_base::servoPinMap[pin];
  if (servoNo < MAX_SERVOS)
    firmata_base::servos[servoNo].write(value);
}
static void
INIT_SERVO_MOTOR(int port)
{
  int pin = port + 8;
  firmata_base::attachServo(pin, 0, 0);
  pinMode(LED_MULTI_FET, OUTPUT);
  digitalWrite(LED_MULTI_FET, HIGH);
}

static void
SERVOMOTOR_SYNCHRONIZED_MOTION(struct servo_sync *ss, int number, byte time)
{
  byte before[8];        // Current angles of servomotor
  double delta[8];       // delta <- target angle - current angle
  byte maxDelta = 0;     // max of delta
  byte calibedDegree;    // Temporary value for calibrating angle.

  // If there are not servomotors setting angle, do nothing.
  if (number == 0) return;

  time = min(max(0, time), 100) / 5; /* 0..100 -> 0..20 */
  time = min(max(0, (20 - time)), 20);

  // Get maximum difference between current angle and target angle.
  for (int i = 0;i < number;i++) {
    int pin = ss[i].port + 8;   /* koov pin no -> firmata pin no. */
    int degree = ss[i].degree;
    int servoNo = firmata_base::servoPinMap[pin];
    before[i] = firmata_base::servos[servoNo].read(); // Current angle.
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
      int servoNo = firmata_base::servoPinMap[pin];
      firmata_base::servos[servoNo].write(calibedDegree);
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
	int servoNo = firmata_base::servoPinMap[pin];
        firmata_base::servos[servoNo].write(before[i]+delta[i]*t);
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

static unsigned int timer_start;
static void
RESET_TIMER()
{

  timer_start = millis();
}

static unsigned int
TIMER()
{

  return (millis() - timer_start) / 1000;
}
