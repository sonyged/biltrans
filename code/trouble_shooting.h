/* -*- mode: c ; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

#include <Wire.h>
#include <MMA8653.h>
#include <Servo.h>

namespace trouble_shooting {

#define BUTTON_A0 24
#define BUTTON_A1 25
#define BUTTON_A2 26
#define BUTTON_A3 27
#define LED_MULTI_RED 13
#define LED_MULTI_GREEN 12
#define LED_MULTI_BLUE 10
#define LED_MULTI_FET 18
#define LED_USB 20
#define LED_BLE 21
#define LED_LIVE 0
#define LED_STDA 1
#define PORT_K0 16
#define PORT_K1 17
#define PORT_K2 24
#define PORT_K3 25
#define PORT_K4 26
#define PORT_K5 27
#define PORT_K6 28
#define PORT_K7 29

#define SERVO_PINS { 2, 3, 6, 7 } /* V2, V3, V4, V5 */
#define LED_PINS { 9, 11 }        /* V7, V8 */
#define BUZZER_PINS { 8 }         /* V6 */
#define BUZZER_INDEX 0
#define DCMOTOR

#define ARRAYCOUNT(a) (sizeof(a) / sizeof((a)[0]))

MMA8653 accel;
int16_t x, y, z;

const byte pinServo[] = SERVO_PINS;
const byte pinLED[] = LED_PINS;
const byte pinBuzzer[] = BUZZER_PINS;
//int delayTime = 4000;
#ifndef FIRMATA_BASE
Servo myservo[ARRAYCOUNT(pinServo)];  //
#else
#define myservo firmata_base::servos
#endif

struct periodic;
struct action {
  int a_interval;               /* interval in msec */
  void (*a_action)(struct periodic *);
  int a_arg;
};

struct periodic {
  const unsigned int p_count;
  const struct action *p_actions;
  boolean p_active;
  uint32_t p_tick;
  unsigned int p_index;
};

static int
periodic_index(struct periodic *p)
{

  return p->p_index % p->p_count;
}

static int
periodic_interval(struct periodic *p)
{
  int cur = periodic_index(p);

  return p->p_actions[cur].a_interval;
}

static int
periodic_arg(struct periodic *p)
{
  int cur = periodic_index(p);

  return p->p_actions[cur].a_arg;
}

static void
periodic_init(struct periodic *p)
{

  p->p_active = true;
  p->p_index = -1;
  p->p_tick = millis() - periodic_interval(p);
}

static void
periodic_process(struct periodic *p)
{
  uint32_t curTick = millis();

  if (!p->p_active)
    return;

  if (curTick - p->p_tick > periodic_interval(p)) {
    p->p_tick = curTick;
    p->p_index++;
    (*p->p_actions[periodic_index(p)].a_action)(p);
  }
}

static void
actionStop(struct periodic *p)
{

  p->p_active = false;
}

static void
actionNop(struct periodic *p)
{

  /* do nothing */;
}

static void dcMotorStop(void);
static void dcMotorBrake(void);
static void dcMotorNormal(int speed);
static void dcMotorReverse(int speed);

static void
dcMotorActionStop(struct periodic *p)
{

  dcMotorStop();
}

static void
dcMotorActionBrake(struct periodic *p)
{

  dcMotorBrake();
}

static void
dcMotorActionNormal(struct periodic *p)
{

  dcMotorNormal(periodic_arg(p));
}

static void
dcMotorActionReverse(struct periodic *p)
{

  dcMotorReverse(periodic_arg(p));
}

struct action dcMotorActions[] = {
  { 3000, dcMotorActionNormal, 255 },
  { 1000, dcMotorActionStop },
  { 3000, dcMotorActionReverse, 255 },
  { 1000, dcMotorActionBrake },
  { 3000, dcMotorActionNormal, 128 },
  { 1000, dcMotorActionStop },
  { 3000, dcMotorActionReverse, 128 },
  { 1000, dcMotorActionBrake },
};

struct periodic dcMotorPeriodic = {
  ARRAYCOUNT(dcMotorActions),
  dcMotorActions,
};

static int servoDegree;
static int (*servoMotorCalcDegree)(struct periodic *p) = 0;

static int
servoMotorDegreeStop(struct periodic *p)
{

  return servoDegree;
}

static int
servoMotorDegreeNormal(struct periodic *p)
{
  uint32_t curTick = millis();

  return (curTick - p->p_tick) * 180 / periodic_interval(p);
}

static int
servoMotorDegreeReverse(struct periodic *p)
{
  uint32_t curTick = millis();

  return 180 -  (curTick - p->p_tick) * 180 / periodic_interval(p);
}

static void
servoMotorActionStop(struct periodic *p)
{

  servoMotorCalcDegree = servoMotorDegreeStop;
}

static void
servoMotorActionNormal(struct periodic *p)
{

  servoMotorCalcDegree = servoMotorDegreeNormal;
}

static void
servoMotorActionReverse(struct periodic *p)
{

  servoMotorCalcDegree = servoMotorDegreeReverse;
}

struct action servoMotorActions[] = {
  { 3000, servoMotorActionNormal },
  { 1000, servoMotorActionStop },
  { 3000, servoMotorActionReverse },
  { 1000, servoMotorActionStop },
};

struct periodic servoMotorPeriodic = {
  ARRAYCOUNT(servoMotorActions),
  servoMotorActions,
};

#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047
#define NOTE_E6  1319
#define NOTE_G6  1568
#define NOTE_C7  2093

static int tonePin()
{

  return pinBuzzer[BUZZER_INDEX % ARRAYCOUNT(pinBuzzer)];
}

static void
toneAction(struct periodic *p)
{

  if (ARRAYCOUNT(pinBuzzer))
    tone(tonePin(), periodic_arg(p));
}

static void
toneNone(struct periodic *p)
{

  if (ARRAYCOUNT(pinBuzzer))
    noTone(tonePin());
}

struct action buzzerActions[] = {
  { 150, toneAction, NOTE_C4, },
  { 150, toneAction, NOTE_E4, },
  { 150, toneAction, NOTE_G4, },
  { 150, toneAction, NOTE_C5, },

  { 150, toneAction, NOTE_C5, },
  { 150, toneAction, NOTE_E5, },
  { 150, toneAction, NOTE_G5, },
  { 150, toneAction, NOTE_C6, },

  { 150, toneAction, NOTE_G5, },
  { 150, toneAction, NOTE_C6, },
  { 150, toneAction, NOTE_E6, },
  { 150, toneAction, NOTE_G6, },
  { 150, toneAction, NOTE_C7, },

  { 150 * 8, toneNone }
};

struct periodic buzzerPeriodic = {
  ARRAYCOUNT(buzzerActions),
  buzzerActions,
};

static const byte multiLedPins[] = {
  LED_MULTI_RED,
  LED_MULTI_GREEN,
  LED_MULTI_BLUE
};

static void
multiLedInit()
{

  pinMode(LED_MULTI_FET, OUTPUT);
  digitalWrite(LED_MULTI_FET, HIGH);

  for (int i = 0; i < ARRAYCOUNT(multiLedPins); i++) {
    pinMode(multiLedPins[i], OUTPUT);
    digitalWrite(multiLedPins[i], HIGH);
  }
}

static void
normalLedOp(const byte pins[], int count, boolean on)
{

  for (int i = 0; i < count; i++) {
    digitalWrite(pins[i], on ? LOW : HIGH);
  }
}

static void
normalLedInit(const byte pins[], int count)
{

  for (int i = 0; i < count; i++) {
    pinMode(pins[i], OUTPUT);
  }
  normalLedOp(pins, count, false);
}

static void
ledInit()
{

  normalLedInit(pinLED, ARRAYCOUNT(pinLED));
  multiLedInit();
}

static void
ledOn()
{

  normalLedOp(pinLED, ARRAYCOUNT(pinLED), true);
}

static void
ledOff()
{

  normalLedOp(pinLED, ARRAYCOUNT(pinLED), false);
}

static void
ledActionOn(struct periodic *p)
{

  ledOn();
}

static void
ledActionOff(struct periodic *p)
{

  ledOff();
}

struct action ledActions[] = {
  { 1000, ledActionOn },
  { 1000, ledActionOff },
};

struct periodic ledPeriodic = {
  ARRAYCOUNT(ledActions),
  ledActions,
};

static void
accelInit()
{

  Wire.begin(); 
  accel.begin(false, 2);
}

static void
accelActionInit(struct periodic *p)
{

  accelInit();
}

struct action accelActions[] = {
  { 1000, accelActionInit },
};

struct periodic accelPeriodic = {
  ARRAYCOUNT(accelActions),
  accelActions,
};

static void
trouble_setup()
{
  for (int i = 0; i < ARRAYCOUNT(pinServo); i++) {
    myservo[i].attach(pinServo[i], 500, 2500);
  }

  ledInit();
  ledOn();

  pinMode(PORT_K6, INPUT);
  pinMode(PORT_K7, INPUT);
  analogReference(AR_DEFAULT);

#if defined(DCMOTOR)
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(12, OUTPUT);
  periodic_init(&dcMotorPeriodic);
#endif

  periodic_init(&servoMotorPeriodic);
  servoDegree = 0;
  servoMotorCalcDegree = servoMotorDegreeStop;

  periodic_init(&buzzerPeriodic);
  periodic_init(&ledPeriodic);
  periodic_init(&accelPeriodic);

  accelInit();
  x = accel.getX();
  y = accel.getY();
  z = accel.getZ();

  SerialUSB.begin(57600);
}  //  setup

enum {
  LED_BLACK = 0,
  LED_RED = 1,
  LED_GREEN = 2,
  LED_YELLOW = 3,
  LED_BLUE = 4,
  LED_MAGENTA = 5,
  LED_CYAN = 6,
  LED_WHITE = 7
};

static void
color_led(int color)
{

  for (int i = 0; i < ARRAYCOUNT(multiLedPins); i++) {
    digitalWrite(multiLedPins[i], (color & (1 << i)) ? LOW : HIGH);
//    analogWrite(multiLedPins[i], (color & (1 << i)) ? 0 : 255);
  }
  digitalWrite(LED_MULTI_FET, color ? LOW : HIGH);
}

static boolean
analog_sensor(int port)
{
  pinMode(port, INPUT_PULLUP);
  delay(0);  //  wait for charging A0 port

  int raw = analogRead(port);
  int val = map(raw, 0, 1023, 0, 255);

  if (val < 64) {
    return false;
  } else {
    return true;
  }
}

static boolean
digital_sensor(int port)
{
  int raw = digitalRead(port);

  if (raw == HIGH) {
    return false;
  } else {
    return true;
  }
}

static void
trouble_loop()
{

  if (SerialUSB.available()) {
    do {
      uint8_t c = SerialUSB.read();
    } while (SerialUSB.available());
    SerialUSB.println(KOOV_VERSION);
  }

  {
    if (digital_sensor(PORT_K2) ||
        digital_sensor(PORT_K3) ||
        digital_sensor(PORT_K4) ||
        digital_sensor(PORT_K5))
      digitalWrite(LED_BLE, LOW);
    else
      digitalWrite(LED_BLE, HIGH);
  }

  boolean live_on = false;
  {
    if (!analog_sensor(PORT_K6))
      live_on = true;
    if (!analog_sensor(PORT_K7))
      live_on = true;
  }

  {
    accel.update();
    int16_t nx = accel.getX(), ny = accel.getY(), nz = accel.getZ();

    x -= nx, y -= ny, z -= nz;
    if (x * x + y * y + z * z > 1000)
      live_on = true;
    x = nx, y = ny, z = nz;
  }

  if (live_on)
    digitalWrite(LED_LIVE, LOW);
  else
    digitalWrite(LED_LIVE, HIGH);

#if defined(DCMOTOR)
  periodic_process(&dcMotorPeriodic);
#endif

  periodic_process(&servoMotorPeriodic);
  {
    int degree = (*servoMotorCalcDegree)(&servoMotorPeriodic);

    if (servoDegree != degree) {
      servoDegree = degree;
      for (int j = 0; j < ARRAYCOUNT(pinServo); j++) {
        myservo[j].write(servoDegree);
      }
    }
  }

  periodic_process(&buzzerPeriodic);
  periodic_process(&ledPeriodic);
  periodic_process(&accelPeriodic);

  delay(20);
}  //  loop

static void
dcMotorNormal(int speed)
{
  digitalWrite(5, LOW);
  analogWrite(4, speed);

  digitalWrite(10, LOW);
  analogWrite(12, speed);
}

static void
dcMotorStop(void)
{
  digitalWrite(5, LOW);
  analogWrite(4, 0);

  digitalWrite(10, LOW);
  analogWrite(12, 0);
}

static void
dcMotorReverse(int speed)
{
  digitalWrite(5, HIGH);
  analogWrite(4, 255 - speed);

  digitalWrite(10, HIGH);
  analogWrite(12, 255 - speed);
}

static void
dcMotorBrake(void)
{
  digitalWrite(5, HIGH);
  analogWrite(4, 255);

  digitalWrite(10, HIGH);
  analogWrite(12, 255);
}

static void
button_setup()
{

  pinMode(BUTTON_A0, INPUT_PULLUP);
  pinMode(BUTTON_A1, INPUT_PULLUP);
  pinMode(BUTTON_A2, INPUT_PULLUP);
  pinMode(BUTTON_A3, INPUT_PULLUP);
}

/*
 * trouble shooting mode prolog
 */
#define PROLOG_LED_INTERVAL (5000/6)

const byte pinLedGroup[][2] = {
  { LED_USB, LED_BLE },
  { LED_STDA, LED_LIVE }
};

static void
prologLedActionToggle(struct periodic *p)
{
  int toggle = periodic_arg(p);

  for (int i = 0; i < ARRAYCOUNT(pinLedGroup); i++) {
    normalLedOp(&pinLedGroup[i][0], 1, toggle);
    normalLedOp(&pinLedGroup[i][1], 1, !toggle);
  }
}

struct action prologLedActions[] = {
  { PROLOG_LED_INTERVAL, prologLedActionToggle, 1 },
  { PROLOG_LED_INTERVAL, prologLedActionToggle, 0 },
};

static int multiLedColor;
static void
prologMultiLedActionToggle(struct periodic *p)
{
  int color = periodic_arg(p);

  multiLedColor = color;
  color_led(color);
}

static int
prologMultiLedPWM(struct periodic *p)
{
  uint32_t tick = millis() - p->p_tick;
  uint32_t interval = periodic_interval(p) / 2;

  if (tick > interval)
    return map(tick - interval, 0, interval, 0, 255); /* ON -> OFF */
  else
    return map(tick, 0, interval, 255, 0); /* OFF -> ON */
}

struct action prologMultiLedActions[] = {
  { PROLOG_LED_INTERVAL, prologMultiLedActionToggle, LED_RED },
  { PROLOG_LED_INTERVAL, prologMultiLedActionToggle, LED_GREEN },
  { PROLOG_LED_INTERVAL, prologMultiLedActionToggle, LED_BLUE },
  { PROLOG_LED_INTERVAL, prologMultiLedActionToggle, LED_YELLOW },
  { PROLOG_LED_INTERVAL, prologMultiLedActionToggle, LED_MAGENTA },
  { PROLOG_LED_INTERVAL, prologMultiLedActionToggle, LED_CYAN },
};

static void (*current_loop)() = 0;

static void
prologExit(struct periodic *p)
{

  for (int i = 0; i < ARRAYCOUNT(pinLedGroup); i++) {
    normalLedInit(pinLedGroup[i], ARRAYCOUNT(pinLedGroup[i]));
  }
  trouble_setup();
  current_loop = trouble_loop;
}

struct action prologExitActions[] = {
  { PROLOG_LED_INTERVAL * 6, actionNop },
  { 0, prologExit },
};

struct periodic prologLedPeriodic[] = {
  { ARRAYCOUNT(prologMultiLedActions), prologMultiLedActions, },
  { ARRAYCOUNT(prologLedActions), prologLedActions, },
  { ARRAYCOUNT(prologExitActions), prologExitActions, },
};

static void
prolog_setup()
{

  for (int i = 0; i < ARRAYCOUNT(pinLedGroup); i++) {
    normalLedInit(pinLedGroup[i], ARRAYCOUNT(pinLedGroup[i]));
  }
  multiLedInit();

  for (int i = 0; i < ARRAYCOUNT(prologLedPeriodic); i++) {
    periodic_init(&prologLedPeriodic[i]);
  }
}

static void
prolog_loop()
{

  for (int i = 0; i < ARRAYCOUNT(prologLedPeriodic); i++) {
    periodic_process(&prologLedPeriodic[i]);
  }

  {
    int pwm = prologMultiLedPWM(&prologLedPeriodic[0]);

    for (int i = 0; i < ARRAYCOUNT(multiLedPins); i++) {
      if ((multiLedColor & (1 << i)) != 0)
        analogWrite(multiLedPins[i], pwm);
    }
  }
}

void
led13_loop()
{
  const byte pin = 13;

  normalLedOp(&pin, 1, 0);
  delay(200);
  normalLedOp(&pin, 1, 1);
  delay(200);
}

void
setup()
{

  button_setup();
  prolog_setup();
  current_loop = prolog_loop;
}

void
loop()
{

  (*current_loop)();
}


#undef BUTTON_A0
#undef BUTTON_A1
#undef BUTTON_A2
#undef BUTTON_A3
#undef LED_MULTI_RED
#undef LED_MULTI_GREEN
#undef LED_MULTI_BLUE
#undef LED_MULTI_FET
#undef LED_USB
#undef LED_BLE
#undef LED_LIVE
#undef LED_STDA
#undef PORT_K0
#undef PORT_K1
#undef PORT_K2
#undef PORT_K3
#undef PORT_K4
#undef PORT_K5
#undef PORT_K6
#undef PORT_K7
#undef SERVO_PINS
#undef LED_PINS
#undef BUZZER_PINS
#undef BUZZER_INDEX
#undef DCMOTOR
#undef ARRAYCOUNT
#undef NOTE_C4
#undef NOTE_E4
#undef NOTE_G4
#undef NOTE_C5
#undef NOTE_E5
#undef NOTE_G5
#undef NOTE_C6
#undef NOTE_E6
#undef NOTE_G6
#undef NOTE_C7
#undef PROLOG_LED_INTERVAL
};
