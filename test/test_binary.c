#include <stdio.h>
#include <stdlib.h>

#include "interp.h"

void
EX_TRACE(const char *msg)
{
}

void
EX_TRACE_INT(int v)
{
}

void
EX_TRACE_HEX(int v)
{
}

int
EX_DELAY(float seconds)
{

  printf("wait %f\n", seconds);
  return ERROR_OK;
}

void
EX_TURN_LED(int port, int mode)
{

  printf("turn-led %d %d\n", port, mode);
}

void
EX_MULTILED(int r, int g, int b)
{

  printf("multi-led %d %d %d\n", r, g, b);
}

void
EX_SET_DCMOTOR_MODE(int port, int direction)
{

  printf("dcmotor-mode %d %d\n", port, direction);
}

void
EX_SET_DCMOTOR_POWER(int port, int power)
{

  printf("dcmotor-power %d %d\n", port, power);
}

void
EX_BUZZER_CONTROL(int port, int mode, int frequency)
{

  if (mode)
    printf("buzzer-on %d %d\n", port, frequency);
  else
    printf("buzzer-off %d\n", port);
}

void
EX_SERVO_MOTOR(int port, int value)
{

  printf("servomotor %d %d\n", port, value);
}

void
EX_SERVOMOTOR_SYNCHRONIZED_MOTION(struct servo_sync *ss, int count, int time)
{

  printf("servomotor-synchronized-motion: time %d\n", time);
  for (int i = 0; i < count; i++)
    printf("ss[%d]: port %d degree %d\n", i, ss[i].port, ss[i].degree);
}

static int digital_value = 0;
int
EX_DIGITAL_SENSOR(int port)
{

  printf("digital-sensor %d\n", port);
  return digital_value++ % 2;
}

static float analog_value = 0;
float EX_ANALOG_SENSOR(int port)
{

  printf("analog-sensor %d\n", port);
  return analog_value += 0.5;
}

int
EX_ACCELEROMETER_VALUE(int port, int dir)
{

  printf("accelerometer %d\n", port);
  return analog_value += 0.5;
}

int
EX_PORT_INIT(int port, ntype part)
{

  printf("port-init %d %x\n", port, part);
  return ERROR_OK;
}

long
EX_RANDOM(long from, long to)
{

  return random() % (to - from) + from;
}


static unsigned int timer_start;
void
EX_RESET_TIMER()
{

  printf("reset-timer\n");
  timer_start = 0;
}

unsigned int
EX_TIMER()
{

  printf("timer %d\n", timer_start);
  return timer_start++;
}


int
main()
{
  uint8_t *p = 0;
  size_t idx = 0, size = 0;
  int c;

  while ((c = getchar()) != EOF) {
    if (idx == size) {
      size_t nsize = size ? size << 1 : 1;
      void *n = realloc(p, nsize);
      if (n == 0)
	exit(1);
      size = nsize;
      p = n;
      idx = size >> 1;
    }
    p[idx++] = c;
  }

  int err = interp_exec(p, idx);
  if (err)
    printf("got error %d\n", err);
}
