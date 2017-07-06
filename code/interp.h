/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

#if !defined(INTERP_H)
#define INTERP_H
int interp_exec(const uint8_t *p, ssize_t size);
int interp_setup(const uint8_t *p, ssize_t size);

/*
 * Type of e_name.
 */
typedef uint16_t ntype;
typedef float vtype;

enum {
  BT_ERROR = 0x00,              /* pseudo type */
  BT_NUMBER = 0x01,
  BT_KEYWORD = 0x02,
  BT_OBJECT = 0x03,
  BT_ARRAY = 0x04,
  BT_INT8 = 0x05,
  BT_INT16 = 0x06,
  BT_INT32 = 0x07,
};

enum {
  ERROR_OK = 0,
  ERROR_FOUND,                  /* 1 */
  ERROR_INTERRUPTED,            /* 2 */
  ERROR_BUFFER_TOO_SHORT,       /* 3 */
  ERROR_ELEMENT_NOT_FOUND,      /* 4 */
  ERROR_UNSUPPORTED,            /* 5 */
  ERROR_NOT_DOCUMENT,           /* 6 */
  ERROR_OVERFLOW,               /* 7 */
  ERROR_INVALID_TYPE,           /* 8 */
  ERROR_INVALID_SIZE,           /* 9 */
  ERROR_OUT_OF_RANGE,           /* 10 */
  ERROR_TOOMANY_SERVO,          /* 11 */
  ERROR_NO_MEMORY,              /* 12 */
  ERROR_INVALID_MAGIC,          /* 13 */
  ERROR_STACK_OVERFLOW,         /* 14 */
};

#if !defined(CHECK_INTR)
#define CHECK_INTR(x)           /* nothing */
#endif

#if 0
#define LIST_POSITION(pos)      (pos) /* 0-origin */
#else
#define LIST_POSITION(pos)      ((pos) - 1) /* 1-origin */
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void EX_TRACE(const char *msg);
void EX_TRACE_INT(int);
void EX_TRACE_HEX(int);
void EX_TRACE_FLOAT(float);
void EX_TRACE_STK(const char *msg, int, int);
#if defined(__cplusplus)
};
#endif

int EX_DELAY(vtype seconds);
long EX_RANDOM(long, long);
void EX_RESET_TIMER();
vtype EX_TIMER();
void EX_TURN_LED(int, int);
void EX_MULTILED(int r, int g, int b); /* 0 <= r/g/b <= 100 */
void EX_BUZZER_CONTROL(int, int, int);
#ifndef servo_sync
#define MAX_SERVOS 8
struct servo_sync {
  uint8_t port;
  uint8_t degree;
};
#define servo_sync servo_sync
extern void *_sbrk(int incr);
#endif
void EX_SERVOMOTOR_SYNCHRONIZED_MOTION(struct servo_sync *, int, int);
void EX_SERVO_MOTOR(int port, int value); /* 0 <= value <= 180 */
void EX_SET_DCMOTOR_MODE(int port, int mode);
void EX_SET_DCMOTOR_POWER(int port, int power);
int EX_DIGITAL_SENSOR(int port);       /* 0 if LOW, 1 otherwise */
vtype EX_ANALOG_SENSOR(int port, int lim); /* 0 <= value <= 100 */
int EX_ACCELEROMETER_VALUE(int, int);
int EX_PORT_INIT(int, ntype);
#endif /* !defined(INTERP_H) */
