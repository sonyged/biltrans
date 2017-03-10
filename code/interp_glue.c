int
EX_DELAY(float seconds)
{

  return delay_with_check(seconds * 1000UL, ERROR_OK, ERROR_INTERRUPTED);
}

void
EX_TRACE(const char *msg)
{

  SerialUSB.println(msg);
}

void
EX_TRACE_INT(int v)
{

  SerialUSB.println(v);
}

void
EX_TRACE_HEX(int v)
{

  SerialUSB.println(v, HEX);
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

unsigned int
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

float
EX_ANALOG_SENSOR(int port)
{

  return sensor_value(port);
}

int
EX_ACCELEROMETER_VALUE(int port, int direction)
{

  return ACCELEROMETER_VALUE(port, direction);
}

int
EX_PORT_INIT(int port, const char *part)
{

  if (strcmp(part, "multi-led") == 0)
    INIT_MULTILED();
  if (strcmp(part, "led") == 0)
    pinMode(port, OUTPUT);
  if (strcmp(part, "dc-motor") == 0)
    INIT_DC_MOTOR(port);
  if (strcmp(part, "servo-motor") == 0)
    INIT_SERVO_MOTOR(port);
  if (strcmp(part, "buzzer") == 0)
    BUZZER_CONTROL(port, OFF, 0);
  if (strcmp(part, "light-sensor") == 0)
    INIT_LIGHT_SENSOR(port);
  if (strcmp(part, "touch-sensor") == 0)
    INIT_TOUCH_SENSOR(port);
  if (strcmp(part, "ir-photo-reflector") == 0)
    INIT_IR_PHOTO_REFLECTOR(port);
  if (strcmp(part, "3-axis-digital-accelerometer") == 0)
    INIT_3_AXIS_DIGITAL_ACCELEROMETER(port ? PORT_K1 : PORT_K0);
  if (strcmp(part, "push-button") == 0)
    INIT_PUSH_BUTTON(port);
  return ERROR_OK;
}

