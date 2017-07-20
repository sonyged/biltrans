/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

#undef ENABLE_TRACING
int
EX_DELAY(vtype seconds)
{

  return delay_with_check(seconds * 1000UL, ERROR_OK, ERROR_INTERRUPTED);
}

void
EX_TRACE(const char *msg)
{

#if defined(ENABLE_TRACING)
  SerialUSB.println(msg);
#endif
}

void
EX_TRACE_INT(int v)
{

#if defined(ENABLE_TRACING)
  SerialUSB.print(" ");
  SerialUSB.println(v);
#endif
}

void
EX_TRACE_HEX(int v)
{

#if defined(ENABLE_TRACING)
  SerialUSB.print(" ");
  SerialUSB.println(v, HEX);
#endif
}

void
EX_TRACE_FLOAT(float v)
{

#if defined(ENABLE_TRACING)
  SerialUSB.print(" ");
  SerialUSB.print(v, 3);
#endif
}

void
EX_TRACE_STK(const char *msg, int p, int q)
{

#if defined(ENABLE_TRACING)
  SerialUSB.print(msg);
  SerialUSB.print(" ");
  SerialUSB.print(p, HEX);
  SerialUSB.print(" ");
  SerialUSB.print(q, HEX);
  SerialUSB.println("");
#endif
}

long
EX_RANDOM(long from, long to)
{

  return random(from, to);
}

void
EX_RESET_TIMER()
{

  RESET_TIMER();
}

vtype
EX_TIMER()
{

  return TIMER();
}

void
EX_TURN_LED(int port, int mode)
{

  TURN_LED(port, mode ? ON : OFF);
}

void
EX_MULTILED(int r, int g, int b)
{

  MULTILED(r, g, b);
}

void
EX_BUZZER_CONTROL(int port, int mode, int freq)
{

  BUZZER_CONTROL(port, mode ? ON : OFF, freq);
}

void
EX_SERVOMOTOR_SYNCHRONIZED_MOTION(struct servo_sync *ss, int count, int time)
{

  SERVOMOTOR_SYNCHRONIZED_MOTION(ss, count, time);
}

void
EX_SERVO_MOTOR(int port, int value)
{

  SERVO_MOTOR(port, value);
}

void
EX_SET_DCMOTOR_MODE(int port, int mode)
{
  dcMotroMode dcMode;

  switch (mode) {
  default:
    return;
  case 0: dcMode = DCMOTOR_NORMAL; break;
  case 1: dcMode = DCMOTOR_REVERSE; break;
  case 2: dcMode = DCMOTOR_COAST; break;
  case 3: dcMode = DCMOTOR_BRAKE; break;
  }

  SET_DCMOTOR_MODE(port, dcMode);
}

void
EX_SET_DCMOTOR_POWER(int port, int power)
{

  SET_DCMOTOR_POWER(port, power);
}

int
EX_DIGITAL_SENSOR(int port)
{

  return digitalRead(port);
}

vtype
EX_ANALOG_SENSOR(int port, int lim)
{
  vtype v = sensor_value(port, lim);

#if 0
  SerialUSB.print(port);
  SerialUSB.print("\t");
  SerialUSB.print(v, 3);
  SerialUSB.println("");
#endif
  return v;
}

int
EX_ACCELEROMETER_VALUE(int port, int direction)
{

  return ACCELEROMETER_VALUE(port, direction);
}

int
EX_PORT_INIT(int port, ntype part)
{

  if (part == Kmulti_led)
    INIT_MULTILED();
  if (part == Kled)
    pinMode(port, OUTPUT);
  if (part == Kdc_motor)
    INIT_DC_MOTOR(port);
  if (part == Kservo_motor)
    INIT_SERVO_MOTOR(port);
  if (part == Kbuzzer)
    BUZZER_CONTROL(port, OFF, 0);
  if (part == Klight_sensor)
    INIT_LIGHT_SENSOR(port);
  if (part == Ksound_sensor)
    INIT_SOUND_SENSOR(port);
  if (part == Ktouch_sensor)
    INIT_TOUCH_SENSOR(port);
  if (part == Kir_photo_reflector)
    INIT_IR_PHOTO_REFLECTOR(port);
  if (part == K3_axis_digital_accelerometer)
    INIT_3_AXIS_DIGITAL_ACCELEROMETER(port ? PORT_K1 : PORT_K0);
  if (part == Kpush_button)
    INIT_PUSH_BUTTON(port);
  return ERROR_OK;
}

#if defined(__cplusplus)
extern "C" {
#endif
extern int __ram_end__ ;
extern int __heap_limit__ ;
#if defined(__cplusplus)
};
#endif

static int elt_used;
static struct elt *elt_freelist;
static struct elt *elt_pool;
static void *stack_bottom;

void
elt_init()
{

  elt_pool = (struct elt *)&__heap_limit__;
  elt_used = 0;
  elt_freelist = 0;
  stack_bottom = (void *)&__ram_end__;
}

int
elt_checkgap(void *stack)
{

  if (stack) {
    if (stack < stack_bottom)
      stack_bottom = stack;
  }
#define STACK_GAP	1024
  if ((int)stack_bottom < (int)&elt_pool[elt_used + 1] + STACK_GAP)
    return 1;
  return 0;
}

void *
elt_alloc(size_t size)
{
  struct elt *e = elt_freelist;

  if (size != sizeof(elt))
    return 0;
  if (e)
    elt_freelist = e->e_next;
  else if (!elt_checkgap(0))
    e = &elt_pool[elt_used++];
  return e;
}

void
elt_free(void *p)
{
  struct elt *e = (struct elt *)p;

  e->e_next = elt_freelist;
  elt_freelist = e;
}
