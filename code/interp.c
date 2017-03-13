#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if !defined(INTERP_H)
#include "interp.h"
#endif
#if !defined(STRING_DICT_H)
#include "string_dict.h"
#endif

#if defined(INT_E_NAME)
#define N_NAME Sname
#define N_BLOCKS Sblocks
#define N_THEN_BLOCKS Sthen_blocks
#define N_ELSE_BLOCKS Selse_blocks
#define N_COUNT Scount
#define N_CONDITION Scondition
#define N_PORT Sport
#define N_MODE Smode
#define N_SECS Ssecs
#define N_POWER Spower
#define N_SPEED Sspeed
#define N_DEGREE Sdegree
#define N_VALUE Svalue
#define N_DIRECTION Sdirection
#define N_FREQUENCY Sfrequency
#define N_OP Sop
#define N_R Sr
#define N_G Sg
#define N_B Sb
#define N_X Sx
#define N_Y Sy
#define N_Z Sz
#define N_FROM Sfrom
#define N_TO Sto
#define N_SET_SERVOMOTOR_DEGREE Sset_servomotor_degree
#define N_FUNCTION Sfunction
#define N_VARIABLE Svariable
#define N_LIST Slist
#define N_ON SON
#define N_OFF SOFF
#define N_NORMAL SNORMAL
#define N_REVERSE SREVERSE
#define N_COAST SCOAST
#define N_BRAKE SBRAKE
#define N_PORT_SETTINGS Sport_settings
#define N_SCRIPTS Sscripts

#define N_V0 SV0
#define N_V1 SV1
#define N_V2 SV2
#define N_V3 SV3
#define N_V4 SV4
#define N_V5 SV5
#define N_V6 SV6
#define N_V7 SV7
#define N_V8 SV8
#define N_V9 SV9

#define N_K0 SK0
#define N_K1 SK1
#define N_K2 SK2
#define N_K3 SK3
#define N_K4 SK4
#define N_K5 SK5
#define N_K6 SK6
#define N_K7 SK7

#define N_A0 SA0
#define N_A1 SA1
#define N_A2 SA2
#define N_A3 SA3
#else
#define N_NAME "name"
#define N_BLOCKS "blocks"
#define N_THEN_BLOCKS "then-blocks"
#define N_ELSE_BLOCKS "else-blocks"
#define N_COUNT "count"
#define N_CONDITION "condition"
#define N_PORT "port"
#define N_MODE "mode"
#define N_SECS "secs"
#define N_POWER "power"
#define N_SPEED "speed"
#define N_DEGREE "degree"
#define N_VALUE "value"
#define N_DIRECTION "direction"
#define N_FREQUENCY "frequency"
#define N_OP "op"
#define N_R "r"
#define N_G "g"
#define N_B "b"
#define N_X "x"
#define N_Y "y"
#define N_Z "z"
#define N_FROM "from"
#define N_TO "to"
#define N_SET_SERVOMOTOR_DEGREE "set-servomotor-degree"
#define N_FUNCTION "function"
#define N_VARIABLE "variable"
#define N_LIST "list"
#define N_ON "ON"
#define N_OFF "OFF"
#define N_NORMAL "NORMAL"
#define N_REVERSE "REVERSE"
#define N_COAST "COAST"
#define N_BRAKE "BRAKE"
#define N_PORT_SETTINGS "port-settings"
#define N_SCRIPTS "scripts"

#define N_V0 "V0"
#define N_V1 "V1"
#define N_V2 "V2"
#define N_V3 "V3"
#define N_V4 "V4"
#define N_V5 "V5"
#define N_V6 "V6"
#define N_V7 "V7"
#define N_V8 "V8"
#define N_V9 "V9"

#define N_K0 "K0"
#define N_K1 "K1"
#define N_K2 "K2"
#define N_K3 "K3"
#define N_K4 "K4"
#define N_K5 "K5"
#define N_K6 "K6"
#define N_K7 "K7"

#define N_A0 "A0"
#define N_A1 "A1"
#define N_A2 "A2"
#define N_A3 "A3"
#endif

typedef float vtype;

static int arg_string(const uint8_t *, ssize_t, ntype, ntype *);
static int arg_u32(const uint8_t *, ssize_t, ntype, uint32_t *);

static int
read32(const uint8_t *end, const ssize_t resid, uint32_t *v)
{
  const uint8_t *p = end - resid;

  if (resid < sizeof(uint32_t))
    return ERROR_BUFFER_TOO_SHORT;
  *v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
  return ERROR_OK;
}

static int
name_equal(ntype x, ntype y)
{
#if defined(INT_E_NAME)
  return x == y;
#else
  return strcmp(x, y) == 0;
#endif
}

static ntype
name_at(const uint8_t *at)
{
#if defined(INT_E_NAME)
  return at[0] | (at[1] << 8);	/* little endian 16-bit unsigned integer */
#else
  return (ntype)at;		/* const char * */
#endif
}

/*
 * Skip type and e_name.
 */
static int
skip_name(const uint8_t *end, ssize_t resid)
{
#if defined(INT_E_NAME)
  return 1 + 2;			/* 8 bit type followed by 16 bit e_name */
#else
  const char *e_name = (const char *)(end - resid) + 1;

  return 1 + strlen(e_name) + 1;
#endif
}

/*
 * Narrow the region down to current elist.
 */
static int
narrow_to_elist(const uint8_t **end, ssize_t *resid, ssize_t *nresid)
{
  uint32_t u32;
  int err;

  err = read32(*end, *resid, &u32);
  if (err)
    return err;
  if (u32 > *resid)
    return ERROR_INVALID_SIZE;
  if (u32 < 5)			/* minimum elist is int32 followed by 0 */
    return ERROR_BUFFER_TOO_SHORT;

  *end = (*end - *resid) + u32;
  *nresid = u32 - 4;		/* skip leading int32 */
  *resid -= u32;
  return ERROR_OK;
}

/*
 * Looking for given name in the elist.
 */
static int
elist_find(const uint8_t *end, ssize_t *resid,
	   int (*compare)(const uint8_t *, ssize_t, void *), void *arg)
{
  ssize_t r = *resid;

  /* There should be at least type and trailing null of e_name. */
  while (r > 1) {
    const uint8_t *p = end - r;
    uint32_t u32;
    int err;

    if ((*compare)(end, r, arg)) {
      *resid = r;
      return ERROR_OK;
    }

    r -= skip_name(end, r);
    switch (*p) {
    case BT_DOUBLE:
      r -= 8;
      break;
    case BT_STRING:
#if defined(INT_E_NAME)
      r -= 2;
#else
      err = read32(end, r, &u32);
      if (err)
	return err;
      r -= 4 + u32;
#endif
      break;
    case BT_DOCUMENT:
    case BT_ARRAY:
      err = read32(end, r, &u32);
      if (err)
	return err;
      r -= u32;
      break;
    case BT_INT32:
      r -= 4;
      break;
    case BT_INT64:
      r -= 8;
      break;

    default:
      return ERROR_UNSUPPORTED;
    }
  }
  return ERROR_ELEMENT_NOT_FOUND;
}

static int
compare_name(const uint8_t *end, ssize_t resid, void *arg)
{
  const uint8_t *p = end - resid;
  ntype e_name = name_at(p + 1);
  ntype name = *(ntype *)arg;

  return name_equal(name, e_name);
}

/*
 * Looking for given name in the elist.
 */
static int
elist_lookup(const uint8_t *end, ssize_t *resid, ntype name)
{

  return elist_find(end, resid, compare_name, (void *)&name);
}

static int
compare_string(const uint8_t *end, ssize_t resid, ntype name, ntype value)
{
  ntype p = 0;

  return arg_string(end, resid, name, &p) == ERROR_OK &&
    name_equal(p, value);
}

typedef struct names {
  ntype n_name;
  ntype n_value;
} names;

static int
compare_names(const uint8_t *end, ssize_t resid, size_t n, const names *names)
{
  const uint8_t *p = end - resid;
  ssize_t nresid;

  if (*p != BT_DOCUMENT)
    return 0;
  resid -= skip_name(end, resid);
  if (narrow_to_elist(&end, &resid, &nresid) != ERROR_OK)
    return 0;
  for (size_t i = 0; i < n; i++)
    if (!compare_string(end, nresid, names[i].n_name, names[i].n_value))
      return 0;
  return 1;
}

static int
compare_function(const uint8_t *end, ssize_t resid, void *arg)
{
  ntype name = (ntype)arg;
  names names[] = {
    { N_NAME, N_FUNCTION },
    { N_FUNCTION, name },
  };

  return compare_names(end, resid, 2, names);
}

typedef struct env {
  void *e_stack;
  vtype e_value;
  const uint8_t *e_end;
  ssize_t e_resid;
  ssize_t e_nvars;
  ssize_t e_nlsts;
  vtype *e_vars;
  void **e_lsts;
} env;
static int exec(env *env, const uint8_t *end, ssize_t *resid);
static int exec_elist(env *env, const uint8_t *end, ssize_t *resid);

static int
arg_string(const uint8_t *end, ssize_t resid, ntype name, ntype *v)
{
  int err;

  err = elist_lookup(end, &resid, name);
  if (err)
    return err;
  if (*(end - resid) != BT_STRING)
    return ERROR_INVALID_TYPE;
  resid -= skip_name(end, resid);
#if !defined(INT_E_NAME)
  resid -= 4;			/* skip string size */
#endif
  if (resid <= 0)
    return ERROR_BUFFER_TOO_SHORT;
  *v = name_at(end - resid);
  //printf("arg_string: %s = %s\n", name, *v);
  return ERROR_OK;
}

static int
arg_u32(const uint8_t *end, ssize_t resid, ntype name, uint32_t *u32)
{
  int err;

  err = elist_lookup(end, &resid, name);
  if (err)
    return err;
  const uint8_t *p = end - resid;
  if (*p != BT_INT32)
    return ERROR_INVALID_TYPE;
  resid -= skip_name(end, resid);
  err = read32(end, resid, u32); /* negative resid is also checked */
  if (err)
    return ERROR_BUFFER_TOO_SHORT;
  //printf("arg_u32: %s = 0x%x\n", name, *u32);
  return ERROR_OK;
}

static int
exec_arg(env *env, const uint8_t *end, ssize_t resid, ntype name)
{
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_arg");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  err = elist_lookup(end, &resid, name);
  if (err)
    return err;
  return exec(env, end, &resid);
}

#define EXEC_BINARY(name)				\
  case S ## name: {					\
    return exec_binary(env, end, nresid, f_ ## name);	\
  }
#define EXEC_UNARY(name)				\
  case S ## name: {					\
    return exec_unary(env, end, nresid, f_ ## name);	\
  }

static int
exec_unary(env *env, const uint8_t *end, ssize_t resid,
	   vtype (*f)(vtype x))
{
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_unary");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  err = exec_arg(env, end, resid, N_X);
  if (err)
    return err;
  env->e_value = (*f)(env->e_value);
  return err;
}

static int
exec_binary(env *env, const uint8_t *end, ssize_t resid,
	    vtype (*f)(vtype x, vtype y))
{
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_binary");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  err = exec_arg(env, end, resid, N_X);
  if (err)
    return err;
  vtype x = env->e_value;
  err = exec_arg(env, end, resid, N_Y);
  if (err)
    return err;
  vtype y = env->e_value;
  env->e_value = (*f)(x, y);
  return err;
}

#define DEFBINARY(name, op) \
  static vtype f_ ## name(vtype x, vtype y) { return x op y; }
DEFBINARY(plus, +);
DEFBINARY(minus, -);
DEFBINARY(multiply, *);
DEFBINARY(divide, /);
DEFBINARY(equal, ==);
DEFBINARY(less_than, <);
DEFBINARY(greater_than, >);

static vtype
f_mod(vtype x, vtype y)
{

  if ((int)y == 0)
    return nanf("");
  return (int)x % (int)y;
}

static vtype
f_and(vtype x, vtype y)
{

  return x != 0 && y != 0;
}

static vtype
f_or(vtype x, vtype y)
{

  return x != 0 || y != 0;
}

static vtype
f_not(vtype x)
{

  return !(x != 0);
}

static vtype
f_round(vtype x)
{

  return round(x);
}

#define deg2rad(x) ((x) / 180.0 * M_PI)

static int
lookup_index(const uint8_t *end, ssize_t resid, uint32_t *u32,
	     ntype name, uint32_t limit)
{
  int err;

  err = arg_u32(end, resid, name, u32);
  if (err)
    return err;
  if (*u32 >= limit)
    return ERROR_OUT_OF_RANGE;
  return ERROR_OK;
}

static int
lookup_variable(env *env, const uint8_t *end, ssize_t resid,
		uint32_t *u32)
{

  return lookup_index(end, resid, u32, N_VARIABLE, env->e_nvars);
}

static int
lookup_list(env *env, const uint8_t *end, ssize_t resid,
	    uint32_t *u32)
{

  return lookup_index(end, resid, u32, N_LIST, env->e_nlsts);
}

static int
with_elist(const uint8_t *end, ssize_t *resid,
	   int (*proc)(const uint8_t *, ssize_t, void *), void *arg)
{
  ssize_t nresid;
  uint32_t u32;
  int err;

  err = narrow_to_elist(&end, resid, &nresid);
  if (err)
    return err;

  return (*proc)(end, nresid, arg);
}

static int
foreach_document(const uint8_t *end, ssize_t resid, int end_of_document,
		 int (*proc)(const uint8_t *end, ssize_t *resid, void *),
		 void *arg)
{
  ssize_t nresid;
  int err;

  err = narrow_to_elist(&end, &resid, &nresid);
  if (err)
    return err;

  for (;;) {
    if (nresid <= 0)
      return ERROR_OVERFLOW;
    if (nresid == 1)		/* trailing nul */
      return end_of_document;
    if (*(end - nresid) != BT_DOCUMENT)
      return ERROR_INVALID_TYPE;
    nresid -= skip_name(end, nresid);
    err = (*proc)(end, &nresid, arg);
    if (err)
      return err;
  }
}

typedef struct lookup_function_args {
  uint32_t idx;
  const uint8_t *end;
  ssize_t resid;
} lookup_function_args;

static int
lookup_function(const uint8_t *end, ssize_t *resid, void *arg)
{
  lookup_function_args *lfa = (lookup_function_args *)arg;
  ntype name;
  uint32_t u32;
  ssize_t nresid;
  int err;

  err = narrow_to_elist(&end, resid, &nresid);
  if (err)
    return err;

  err = arg_string(end, nresid, N_NAME, &name);
  if (err)
    return err;

  if (!name_equal(name, N_FUNCTION))
    return ERROR_OK;

  err = arg_u32(end, nresid, N_FUNCTION, &u32);
  if (err)
    return err;

  if (u32 != lfa->idx)
    return ERROR_OK;

  lfa->end = end;
  lfa->resid = nresid;
  return ERROR_FOUND;
}

static int
exec_function(env *env, uint32_t idx)
{
  lookup_function_args lfa;
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_function");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  lfa.idx = idx;
  err = foreach_document(env->e_end, env->e_resid, ERROR_ELEMENT_NOT_FOUND,
			 lookup_function, &lfa);
  if (err != ERROR_FOUND)
    return err;

  return exec_arg(env, lfa.end, lfa.resid, N_BLOCKS);
}

static int
mode_value(ntype mode)
{
  if (name_equal(mode, N_ON))
    return 1;
  return 0;
}

static int
dcmode_value(ntype mode)
{
  if (name_equal(mode, N_NORMAL))
    return 0;
  if (name_equal(mode, N_REVERSE))
    return 1;
  if (name_equal(mode, N_COAST))
    return 2;
  if (name_equal(mode, N_BRAKE))
    return 3;
  return 0;
}

static int
acceldir_value(ntype mode)
{
  if (name_equal(mode, N_X))
    return 1;
  if (name_equal(mode, N_Y))
    return 2;
  if (name_equal(mode, N_Z))
    return 3;
  return 1;
}

static int
port_value(ntype port)
{
  static const struct {
    ntype port;
    int value;
  } pv[] = {
    { N_V0, 0 },
    { N_V1, 1 },

    { N_V2, 2 },
    { N_V3, 3 },
    { N_V4, 6 },
    { N_V5, 7 },
    { N_V6, 8 },
    { N_V7, 9 },
    { N_V8, 11 },
    { N_V9, 13 },

    { N_K0, 0 },
    { N_K1, 1 },

    { N_K2, 24 },
    { N_K3, 25 },
    { N_K4, 26 },
    { N_K5, 27 },
    { N_K6, 28 },
    { N_K7, 29 },

    { N_A0, 24 },
    { N_A1, 25 },
    { N_A2, 26 },
    { N_A3, 27 },
  };

  for (int i = 0; i < sizeof(pv) / sizeof(pv[0]); i++) {
    if (name_equal(pv[i].port, port))
      return pv[i].value;
  }
  return 0;
}

static int
setup_ss(env *env, const uint8_t *end, ssize_t *resid, struct servo_sync *ss)
{
  ntype port = 0;
  ntype name;
  uint32_t u32;
  ssize_t nresid;
  int err;

  err = narrow_to_elist(&end, resid, &nresid);
  if (err)
    return err;

  err = arg_string(end, nresid, N_NAME, &name);
  if (err)
    return err;

  if (!name_equal(name, N_SET_SERVOMOTOR_DEGREE))
    return ERROR_INVALID_TYPE;

  err = arg_string(end, nresid, N_PORT, &port);
  if (err)
    return err;

  err = exec_arg(env, end, nresid, N_DEGREE);
  if (err)
    return err;

  ss->port = port_value(port);
  ss->degree = env->e_value;
  return ERROR_OK;
}

static int
init_servo_sync(env *env, const uint8_t *end, ssize_t resid,
		struct servo_sync *ss, size_t *count)
{
  ssize_t nresid;
  const size_t max_count = *count;
  int err;

  *count = 0;
  err = elist_lookup(end, &resid, N_BLOCKS);
  if (err)
    return err;
  if (*(end - resid) != BT_ARRAY)
    return ERROR_INVALID_TYPE;
  resid -= skip_name(end, resid);
  err = narrow_to_elist(&end, &resid, &nresid);
  if (err)
    return err;
  for (;;) {
    if (nresid <= 0)
      return ERROR_OVERFLOW;
    if (nresid == 1)		/* trailing nul */
      return ERROR_OK;
    if (*(end - nresid) != BT_DOCUMENT)
      return ERROR_INVALID_TYPE;
    if (*count == max_count)
      return ERROR_TOOMANY_SERVO;
    nresid -= skip_name(end, nresid);
    err = setup_ss(env, end, &nresid, &ss[(*count)++]);
    if (err)
      return err;
  }
}

static int
exec_block(env *env, const uint8_t *end, ssize_t *resid)
{
  ssize_t nresid;
  ntype name;
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_block");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  err = narrow_to_elist(&end, resid, &nresid);
  if (err)
    return err;

  err = arg_string(end, nresid, N_NAME, &name);
  if (err)
    return err;

  switch (name) {
  case Swhen_green_flag_clicked: {
    return exec_arg(env, end, nresid, N_BLOCKS);
  }

  case Srepeat: {
    err = exec_arg(env, end, nresid, N_COUNT);
    if (err)
      return err;
    ssize_t count = env->e_value;
    while (count-- > 0) {
      CHECK_INTR(ERROR_INTERRUPTED);
      err = exec_arg(env, end, nresid, N_BLOCKS);
      if (err)
	return err;
    }
    return ERROR_OK;
  }

  case Srepeat_until: {
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
      err = exec_arg(env, end, nresid, N_CONDITION);
      if (err)
	return err;
      if (env->e_value != 0)
	break;
      err = exec_arg(env, end, nresid, N_BLOCKS);
      if (err)
	return err;
    }
    return ERROR_OK;
  }

  case Swait_until: {
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
      err = exec_arg(env, end, nresid, N_CONDITION);
      if (err)
	return err;
      if (env->e_value != 0)
	break;
      err = EX_DELAY(0.02);
      if (err)
	return err;
    }
    return ERROR_OK;
  }

  case Sforever: {
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
      err = exec_arg(env, end, nresid, N_BLOCKS);
      if (err)
	return err;
    }
    return ERROR_OK;
  }

  case Sif_then: {
    err = exec_arg(env, end, nresid, N_CONDITION);
    if (err)
      return err;
    if (env->e_value != 0)
      return exec_arg(env, end, nresid, N_BLOCKS);
    return ERROR_OK;
  }

  case Sif_then_else: {
    err = exec_arg(env, end, nresid, N_CONDITION);
    if (err)
      return err;
    if (env->e_value != 0)
      return exec_arg(env, end, nresid, N_THEN_BLOCKS);
    return exec_arg(env, end, nresid, N_ELSE_BLOCKS);
  }

  case Swait: {
    err = exec_arg(env, end, nresid, N_SECS);
    if (err)
      return err;
    return EX_DELAY(env->e_value);
  }

    EXEC_BINARY(plus);
    EXEC_BINARY(minus);
    EXEC_BINARY(multiply);
    EXEC_BINARY(divide);
    EXEC_BINARY(mod);

    EXEC_BINARY(and);
    EXEC_BINARY(or);
    EXEC_BINARY(equal);
    EXEC_BINARY(less_than);
    EXEC_BINARY(greater_than);

    EXEC_UNARY(not);
    EXEC_UNARY(round);

  case Smath: {
    ntype op;
    err = arg_string(end, nresid, N_OP, &op);
    if (err)
      return err;
    err = exec_arg(env, end, nresid, N_X);
    if (err)
      return err;
    switch (op) {
    case Sabs:
#if 1
      env->e_value = fabsf(env->e_value);
#else
      if (env->e_value < 0)
	env->e_value = -env->e_value;
#endif
      break;
    case Ssqrt:
      env->e_value = sqrtf(env->e_value);
      break;
    case Ssin:
      env->e_value = sinf(deg2rad(env->e_value));
      break;
    case Scos:
      env->e_value = cosf(deg2rad(env->e_value));
      break;
    case Stan:
#if 0
      env->e_value = tanf(deg2rad(env->e_value));
#else
      {
	const vtype cv = cosf(deg2rad(env->e_value));
	const vtype sv = sinf(deg2rad(env->e_value));
	if (fabsf(cv) == 0)
	  env->e_value = copysignf(INFINITY, sv);
	else
	  env->e_value = sv / cv;
      }
#endif
      break;
    case Sln:
      env->e_value = logf(env->e_value);
      break;
    case Slog:
#if 0
      env->e_value = log10f(env->e_value);
#else
      env->e_value = logf(env->e_value) / M_LN10;
#endif
      break;
    case Sec:			/* e^ */
#if 0
      env->e_value = expf(env->e_value);
#else
      env->e_value = powf(M_E, env->e_value);
#endif
      break;
    case S10c:			/* 10^ */
#if 1
      env->e_value = powf(10, env->e_value);
#else
      env->e_value = exp10f(env->e_value);
#endif
      break;
    default:
      return ERROR_UNSUPPORTED;
    }
    return ERROR_OK;
  }

  case Sturn_led: {
    ntype port = 0;
    ntype mode = 0;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = arg_string(end, nresid, N_MODE, &mode);
    if (err)
      return err;
    EX_TURN_LED(port_value(port), mode_value(mode));
    return ERROR_OK;
  }

  case Smulti_led: {
    vtype r, g, b;
    err = exec_arg(env, end, nresid, N_R);
    if (err)
      return err;
    r = env->e_value;
    err = exec_arg(env, end, nresid, N_G);
    if (err)
      return err;
    g = env->e_value;
    err = exec_arg(env, end, nresid, N_B);
    if (err)
      return err;
    b = env->e_value;
    EX_MULTILED(r, g, b);
    return ERROR_OK;
  }

  case Sturn_dcmotor_on: {
    ntype port = 0;
    ntype direction = 0;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = arg_string(end, nresid, N_DIRECTION, &direction);
    if (err)
      return err;
    EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(direction));
    return ERROR_OK;
  }

  case Sturn_dcmotor_off: {
    ntype port = 0;
    ntype mode = 0;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = arg_string(end, nresid, N_MODE, &mode);
    if (err)
      return err;
    EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(mode));
    return ERROR_OK;
  }

  case Sset_dcmotor_power: {
    ntype port = 0;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = exec_arg(env, end, nresid, N_POWER);
    if (err)
      return err;
    EX_SET_DCMOTOR_POWER(port_value(port), env->e_value);
    return ERROR_OK;
  }

  case Sbuzzer_on: {
    ntype port = 0;
    uint32_t u32;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = exec_arg(env, end, nresid, N_FREQUENCY);
    if (err)
      return err;
    EX_BUZZER_CONTROL(port_value(port), 1, env->e_value);
    return ERROR_OK;
  }

  case Sbuzzer_off: {
    ntype port = 0;
    uint32_t u32;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    EX_BUZZER_CONTROL(port_value(port), 0, 0);
    return ERROR_OK;
  }

  case Sset_servomotor_degree: {
    ntype port = 0;
    uint32_t u32;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = exec_arg(env, end, nresid, N_DEGREE);
    if (err)
      return err;
    EX_SERVO_MOTOR(port_value(port), env->e_value);
    return ERROR_OK;
  }

  case Sservomotor_synchronized_motion: {
    err = exec_arg(env, end, nresid, N_SPEED);
    if (err)
      return err;
    const int time = env->e_value;
    struct servo_sync ss[MAX_SERVOS];
    size_t count = sizeof(ss) / sizeof(ss[0]);
    err = init_servo_sync(env, end, nresid, ss, &count);
    if (err)
      return err;
    EX_SERVOMOTOR_SYNCHRONIZED_MOTION(ss, count, time);
    return ERROR_OK;
  }

  case Sir_photo_reflector_value:
  case Slight_sensor_value: {
    ntype port = 0;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    env->e_value = EX_ANALOG_SENSOR(port_value(port));
    return ERROR_OK;
  }

  case S3_axis_digital_accelerometer_value: {
    ntype port = 0;
    ntype dir = 0;

    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = arg_string(end, nresid, N_DIRECTION, &dir);
    if (err)
      return err;
    env->e_value = EX_ACCELEROMETER_VALUE(port_value(port),
					  acceldir_value(dir));
    return ERROR_OK;
  } 

  case Sbutton_value:
  case Stouch_sensor_value: {
    ntype port = 0;
    ntype mode = 0;
    err = arg_string(end, nresid, N_PORT, &port);
    if (err)
      return err;
    err = arg_string(end, nresid, N_MODE, &mode);
    if (err)
      return err;
    if (name_equal(mode, N_ON))
      env->e_value = EX_DIGITAL_SENSOR(port_value(port)) == 0;
    else
      env->e_value = EX_DIGITAL_SENSOR(port_value(port)) != 0;
    return ERROR_OK;
  }

  case Svariable_ref: {
    uint32_t u32;

    err = lookup_variable(env, end, nresid, &u32);
    if (err)
      return err;
    env->e_value = env->e_vars[u32];
    return ERROR_OK;
  }

  case Sset_variable_to:
  case Schange_variable_by: {
    const int assign = name == Sset_variable_to;
    uint32_t u32;

    err = lookup_variable(env, end, nresid, &u32);
    if (err)
      return err;
    err = exec_arg(env, end, nresid, N_VALUE);
    if (err)
      return err;
    if (assign)
      env->e_vars[u32] = env->e_value;
    else
      env->e_vars[u32] += env->e_value;
    return ERROR_OK;
  }

  case Scall_function: {
    uint32_t u32;

    err = arg_u32(end, nresid, N_FUNCTION, &u32);
    if (err)
      return err;
    return exec_function(env, u32);
  }

  case Spick_random: {
    err = exec_arg(env, end, nresid, N_FROM);
    if (err)
      return err;
    const vtype from = env->e_value;
    err = exec_arg(env, end, nresid, N_TO);
    if (err)
      return err;
    const vtype to = env->e_value;
    env->e_value = EX_RANDOM(from, to + 1);
    return ERROR_OK;
  }

  case Sbreakpoint:
  case Sfunction:
  case Svariable:
  case Slist:
    return ERROR_OK;		/* NOP */

  case Sreset_timer: {
    EX_RESET_TIMER();
    return ERROR_OK;
  }
  case Stimer: {
    env->e_value = EX_TIMER();
    return ERROR_OK;
  }


  /*
    3-axis-digital-accelerometer-value
    less-than-or-equal?
    greater-than-or-equal?
    list-length
    list-add
    list-contains?
    list-ref
    list-delete
    list-replace
    list-insert
   */

  default:
    return ERROR_UNSUPPORTED;
  }
}

/*
 * Evaluate single element.
 */
static int
exec(env *env, const uint8_t *end, ssize_t *resid)
{
  const uint8_t *p = end - *resid;
  uint32_t u32;
  int err;
  union {
    double d;
    int32_t i32;
    int64_t i64;
    uint8_t b[8];
  } u;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  if (resid <= 0)
    return ERROR_OVERFLOW;

  switch (*p) {
  case BT_DOCUMENT:
    *resid -= skip_name(end, *resid);
    return exec_block(env, end, resid);
  case BT_ARRAY:
    *resid -= skip_name(end, *resid);
    return exec_elist(env, end, resid);
  case BT_DOUBLE:
    *resid -= skip_name(end, *resid);
    for (int i = 0; i < 8; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.d;		/* convert from double to vtype */
    *resid -= 8;
    return ERROR_OK;
  case BT_INT32:
    *resid -= skip_name(end, *resid);
    for (int i = 0; i < 4; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.i32;	/* convert from 32 bit integer to vtype */
    *resid -= 4;
    return ERROR_OK;
  case BT_INT64:
    *resid -= skip_name(end, *resid);
    for (int i = 0; i < 8; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.i64;	/* convert from 64 bit integer to vtype */
    *resid -= 8;
    return ERROR_OK;
  default:
    return ERROR_UNSUPPORTED;
  }
  return ERROR_OK;
}

static int
exec_elist(env *env, const uint8_t *end, ssize_t *resid)
{
  ssize_t nresid;
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_elist");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }
  err = narrow_to_elist(&end, resid, &nresid);
  if (err)
    return err;

  for (;;) {
    if (nresid <= 0)
      return ERROR_OVERFLOW;
    if (nresid == 1)		/* trailing nul */
      return ERROR_OK;
    err = exec(env, end, &nresid);
    if (err)
      return err;
  }
}

static int
port_init(const uint8_t *end, ssize_t *resid)
{
  const uint8_t *p = end - *resid;
  int err;

  if (resid <= 0)
    return ERROR_OVERFLOW;
  if (*p != BT_STRING)
    return ERROR_INVALID_TYPE;
  ntype port = name_at(p + 1);
  *resid -= skip_name(end, *resid);
#if !defined(INT_E_NAME)
  uint32_t u32;
  err = read32(end, *resid, &u32);
  if (err)
	return err;
  *resid -= 4;
#endif
  ntype part = name_at(end - *resid);
#if defined(INT_E_NAME)
  *resid -= 2;
#else
  *resid -= u32;
#endif
  err = EX_PORT_INIT(port_value(port), part);
  return ERROR_OK;
}

static int
setup_ports(const uint8_t *end, ssize_t resid)
{
  ssize_t nresid;
  int err;

  err = elist_lookup(end, &resid, N_PORT_SETTINGS);
  switch (err) {
  case ERROR_OK:
    if (*(end - resid) != BT_DOCUMENT)
      return ERROR_INVALID_TYPE;
    resid -= skip_name(end, resid);
    err = narrow_to_elist(&end, &resid, &nresid);
    if (err)
      return err;
    for (;;) {
      if (nresid <= 0)
	return ERROR_OVERFLOW;
      if (nresid == 1)		/* trailing nul */
	return ERROR_OK;
      err = port_init(end, &nresid);
      if (err)
	return err;
    }
    break;
  case ERROR_ELEMENT_NOT_FOUND:
    err = ERROR_OK;
    break;
  default:
    break;
  }
  return err;
}

typedef struct parse_fvl_args {
  int *n_vars;
  int *n_lsts;
} parse_fvl_args;

static int
parse_fvl(const uint8_t *end, ssize_t *resid, void *arg)
{
  parse_fvl_args *pfa = (parse_fvl_args *)arg;
  ssize_t nresid;
  ntype name;
  int err;

  err = narrow_to_elist(&end, resid, &nresid);
  if (err)
    return err;

  err = arg_string(end, nresid, N_NAME, &name);
  if (err)
    return err;

  switch (name) {
  case Svariable:
    (*pfa->n_vars)++;
    return ERROR_OK;
  case Slist:
    (*pfa->n_lsts)++;
    return ERROR_OK;
  default:
    return ERROR_OK;
  }
}

static int
count_environ(const uint8_t *end, ssize_t resid, int *n_vars, int *n_lsts)
{
  parse_fvl_args pfa;

  pfa.n_vars = n_vars;
  pfa.n_lsts = n_lsts;
  *pfa.n_vars = *pfa.n_lsts = 0;
  return foreach_document(end, resid, ERROR_OK, parse_fvl, &pfa);
}

static int
exec_script(const uint8_t *end, ssize_t *resid)
{
  env e_store, *env = &e_store;
  int err, n_vars, n_lsts;

  EX_TRACE_HEX((int)&err);
  err = setup_ports(end, *resid);
  if (err)
    return err;

  err = elist_lookup(end, resid, N_SCRIPTS);
  if (err)
    return err;
  if (*(end - *resid) != BT_ARRAY)
    return ERROR_UNSUPPORTED;

  //EX_TRACE("script found");
  /*
   * At this point, we're looking at:
   *   "\x04" "scripts\x00" document
   *      ||
   *   "\x04" "scripts\x00" int32 e_list "\x00" 
   */
  *resid -= skip_name(end, *resid);

  /* Now, we're looking at (e_list is an array of blocks):
   *   int32 e_list "\x00" 
   */

  err = count_environ(end, *resid, &n_vars, &n_lsts);
  if (err)
    return err;
  //printf("variable = %d, list = %d\n", n_vars, n_lsts);

  env->e_stack = &err;
  env->e_end = end;
  env->e_resid = *resid;
  env->e_nvars = n_vars;
  env->e_nlsts = n_lsts;
  env->e_vars = (vtype *)alloca(sizeof(vtype) * n_vars);
  for (size_t i = 0; i < n_vars; i++)
    env->e_vars[i] = 0;
  env->e_lsts = (void **)alloca(sizeof(void *) * n_lsts);
  for (size_t i = 0; i < n_lsts; i++)
    env->e_lsts[i] = 0;
  //printf("e_vars: %p, e_lsts: %p\n", env->e_vars, env->e_lsts);
  return exec_elist(env, end, resid);
}

int
interp_exec(const uint8_t *p, ssize_t size)
{
  int err;
  const uint8_t *end = p + size;
  ssize_t resid = size;
  uint32_t u32;
  extern uint32_t __koov_data_start__;
  extern uint32_t __koov_data_end__;
  extern uint32_t __end__;
  extern uint32_t __HeapLimit;
  extern uint32_t __StackTop;
  extern uint32_t __StackLimit;

#if 0
  EX_TRACE("interp: start");
  EX_TRACE_HEX((int)&__koov_data_start__);
  EX_TRACE_HEX((int)&__koov_data_end__);
  EX_TRACE_HEX((int)&__end__);
  EX_TRACE_HEX((int)&__HeapLimit);
  EX_TRACE_HEX((int)&__StackTop);
  EX_TRACE_HEX((int)&__StackLimit);
  EX_TRACE_HEX((int)&err);
#endif
  //EX_TRACE_HEX((int)p);
  //EX_TRACE_HEX(p[0]);
  //EX_TRACE_HEX(p[1]);
  const uint32_t *q = (const uint32_t *)0xce00;
  //EX_TRACE_HEX(q[0]);
  const char *r = (const char *)alloca(100);

  /*
   * int32 e_list "\x00"
   */
  err = read32(end, resid, &u32);
  if (err) {
    //EX_TRACE("interp: failed to read size");
    return err;
  }
  if (u32 != size) {
    //EX_TRACE("interp: size mismatch");
    //EX_TRACE_INT(u32);
    //EX_TRACE_INT(size);
    return ERROR_BUFFER_TOO_SHORT;
  }
  if (size < 5) {
    //EX_TRACE("interp: size too small");
    return ERROR_BUFFER_TOO_SHORT;
  }

  end -= 1;			/* trailing 0 */
  resid -= 5;			/* leading int32 + trailing 0 */
  return exec_script(end, &resid);
}
