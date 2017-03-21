#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if !defined(INTERP_H)
#include "interp.h"
#endif
#if !defined(KEYWORD_DICT_H)
#include "keyword_dict.h"
#endif

typedef float vtype;

static int arg_keyword(const uint8_t *, ssize_t, int, ntype, ntype *);
static int arg_int(const uint8_t *, ssize_t, int, ntype, int32_t *);

#undef ELIST_NUL
#if defined(ELIST_NUL)
#define ELIST_SIZE(s)	((s) + 1)
#else
#define ELIST_SIZE(s)	((s) + 0)
#endif

static int
read32(const uint8_t *end, const ssize_t resid, uint32_t *v)
{
  const uint8_t *p = end - resid;

#define SIZE16
#if !defined(SIZE16)
  if (resid < sizeof(uint32_t))
    return ERROR_BUFFER_TOO_SHORT;
  *v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
#else
  if (resid < sizeof(uint16_t))
    return ERROR_BUFFER_TOO_SHORT;
  *v = p[0] | (p[1] << 8);
#endif
  return ERROR_OK;
}

static int
name_equal(ntype x, ntype y)
{

  return x == y;
}

static ntype
name_at(const uint8_t *at)
{

  return at[0] | (at[1] << 8);	/* little endian 16-bit unsigned integer */
}

/*
 * Skip type and e_name.
 */
static int
skip_name(const uint8_t *end, ssize_t resid, int array)
{

  if (array)
    return 1;
  return 1 + 2;			/* 8 bit type followed by 16 bit e_name */
}

#define CALL(F, ...) do {						\
  int err = (F)(__VA_ARGS__);						\
  if (err) {								\
    /* fprintf(stderr, #F ": %d: err = %d\n", __LINE__, err); */	\
    return err;								\
  }									\
} while (0)

/*
 * Narrow the region down to current elist.
 */
static int
narrow_to_elist(const uint8_t **end, ssize_t *resid, ssize_t *nresid)
{
  uint32_t u32;
#if !defined(SIZE16)
  const size_t size = 4;
#else
  const size_t size = 2;
#endif
  int err;

  CALL(read32, *end, *resid, &u32);
  if (u32 > *resid)
    return ERROR_INVALID_SIZE;
  if (u32 < ELIST_SIZE(size))	    /* minimum elist is int32 followed by 0 */
    return ERROR_BUFFER_TOO_SHORT;

  *end = (*end - *resid) + u32;
  *nresid = u32 - size;		/* skip leading int32 */
  *resid -= u32;
  return ERROR_OK;
}

/*
 * Looking for given name in the elist.
 */
static int
elist_find(const uint8_t *end, ssize_t *resid, int array,
	   int (*compare)(const uint8_t *, ssize_t, void *), void *arg)
{
  ssize_t r = *resid;

  /* There should be at least type and trailing null of e_name. */
  while (r > ELIST_SIZE(0)) {
    const uint8_t *p = end - r;
    uint32_t u32;
    int err;

    if ((*compare)(end, r, arg)) {
      *resid = r;
      return ERROR_OK;
    }

    r -= skip_name(end, r, array);
    switch (*p) {
    case BT_NUMBER:
      r -= 4;
      break;
    case BT_KEYWORD:
      r -= 2;
      break;
    case BT_DOCUMENT:
    case BT_ARRAY:
      CALL(read32, end, r, &u32);
      r -= u32;
      break;
    case BT_INT8:
      r -= 1;
      break;
    case BT_INT16:
      r -= 2;
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
elist_lookup(const uint8_t *end, ssize_t *resid, int array, ntype name)
{

  return elist_find(end, resid, array, compare_name, (void *)&name);
}

static int
compare_keyword(const uint8_t *end, ssize_t resid, int array,
		ntype name, ntype value)
{
  ntype p = 0;

  return arg_keyword(end, resid, array, name, &p) == ERROR_OK &&
    name_equal(p, value);
}

typedef struct names {
  ntype n_name;
  ntype n_value;
} names;

static int
compare_names(const uint8_t *end, ssize_t resid, int array,
	      size_t n, const names *names)
{
  const uint8_t *p = end - resid;
  ssize_t nresid;

  if (*p != BT_DOCUMENT)
    return 0;
  resid -= skip_name(end, resid, array);
  if (narrow_to_elist(&end, &resid, &nresid) != ERROR_OK)
    return 0;
  for (size_t i = 0; i < n; i++)
    if (!compare_keyword(end, nresid, 0, names[i].n_name, names[i].n_value))
      return 0;
  return 1;
}

static int
compare_function(const uint8_t *end, ssize_t resid, int array, void *arg)
{
  ntype name = *(ntype *)arg;
  names names[] = {
    { Kname, Kfunction },
    { Kfunction, name },
  };

  return compare_names(end, resid, array, 2, names);
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
static int exec(env *env, const uint8_t *end, ssize_t *resid, int array);
static int exec_array(env *env, const uint8_t *end, ssize_t *resid);

static int
arg_keyword(const uint8_t *end, ssize_t resid, int array, ntype name, ntype *v)
{
  int err;

  CALL(elist_lookup, end, &resid, array, name);
  if (*(end - resid) != BT_KEYWORD)
    return ERROR_INVALID_TYPE;
  resid -= skip_name(end, resid, array);
  if (resid <= 0)
    return ERROR_BUFFER_TOO_SHORT;
  *v = name_at(end - resid);
  //printf("arg_keyword: %s = %s\n", name, *v);
  return ERROR_OK;
}

static int
arg_int(const uint8_t *end, ssize_t resid, int array, ntype name, int32_t *i32)
{
  int err;

  CALL(elist_lookup, end, &resid, array, name);
  const uint8_t *p = end - resid;
  resid -= skip_name(end, resid, array);
  const uint8_t *q = end - resid;
  switch (*p) {
  case BT_INT8:
    if (resid < 1)
      return ERROR_BUFFER_TOO_SHORT;
    *i32 = (int8_t)q[0];
    break;
  case BT_INT16:
    if (resid < 2)
      return ERROR_BUFFER_TOO_SHORT;
    *i32 = (int16_t)(q[0] | (q[1] << 8));
    break;
  case BT_INT32:
    if (resid < 4)
      return ERROR_BUFFER_TOO_SHORT;
    *i32 = (int32_t)(q[0] | (q[1] << 8) | (q[0] << 16) | (q[1] << 24));
  default:
    return ERROR_INVALID_TYPE;
  }
  //printf("arg_int: %s = 0x%x\n", name, *u32);
  return ERROR_OK;
}

static int
exec_arg(env *env, const uint8_t *end, ssize_t resid, int array, ntype name)
{
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_arg");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  CALL(elist_lookup, end, &resid, array, name);
  CALL(exec, env, end, &resid, 0); /* execute single block */
  return ERROR_OK;
}

#define EXEC_BINARY(name)					\
  case K ## name: {						\
    return exec_binary(env, end, nresid, array, f_ ## name);	\
  }
#define EXEC_UNARY(name)					\
  case K ## name: {						\
    return exec_unary(env, end, nresid, array, f_ ## name);	\
  }

static int
exec_unary(env *env, const uint8_t *end, ssize_t resid, int array,
	   vtype (*f)(vtype x))
{
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_unary");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  CALL(exec_arg, env, end, resid, array, Kx);
  env->e_value = (*f)(env->e_value);
  return ERROR_OK;
}

static int
exec_binary(env *env, const uint8_t *end, ssize_t resid, int array,
	    vtype (*f)(vtype x, vtype y))
{
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_binary");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  CALL(exec_arg, env, end, resid, array, Kx);
  vtype x = env->e_value;
  CALL(exec_arg, env, end, resid, array, Ky);
  vtype y = env->e_value;
  env->e_value = (*f)(x, y);
  return ERROR_OK;
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
  int32_t i32;

  CALL(arg_int, end, resid, 0, name, &i32);
  if (i32 < 0 || i32 >= limit)
    return ERROR_OUT_OF_RANGE;
  *u32 = i32;
  return ERROR_OK;
}

static int
lookup_variable(env *env, const uint8_t *end, ssize_t resid,
		uint32_t *u32)
{

  return lookup_index(end, resid, u32, Kvariable, env->e_nvars);
}

static int
lookup_list(env *env, const uint8_t *end, ssize_t resid,
	    uint32_t *u32)
{

  return lookup_index(end, resid, u32, Klist, env->e_nlsts);
}

static int
with_elist(const uint8_t *end, ssize_t *resid, int array,
	   int (*proc)(const uint8_t *, ssize_t, int, void *), void *arg)
{
  ssize_t nresid;
  uint32_t u32;

  CALL(narrow_to_elist, &end, resid, &nresid);
  return (*proc)(end, nresid, array, arg);
}

static int
foreach_document(const uint8_t *end, ssize_t resid, int array,
		 int end_of_document,
		 int (*proc)(const uint8_t *end, ssize_t *resid, int array,
			     void *),
		 void *arg)
{
  ssize_t nresid;
  int err;

  CALL(narrow_to_elist, &end, &resid, &nresid);

  for (;;) {
    if (nresid < ELIST_SIZE(0))
      return ERROR_OVERFLOW;
    if (nresid == ELIST_SIZE(0)) /* trailing nul */
      return end_of_document;
    const int type = *(end - nresid);
    if (type != BT_DOCUMENT)
      return ERROR_INVALID_TYPE;
    nresid -= skip_name(end, nresid, array);
    CALL((*proc), end, &nresid, 0, arg);
  }
}

typedef struct lookup_function_args {
  uint32_t idx;
  const uint8_t *end;
  ssize_t resid;
} lookup_function_args;

static int
lookup_function(const uint8_t *end, ssize_t *resid, int array, void *arg)
{
  lookup_function_args *lfa = (lookup_function_args *)arg;
  ntype name;
  int32_t i32;
  ssize_t nresid;
  int err;

  CALL(narrow_to_elist, &end, resid, &nresid);
  CALL(arg_keyword, end, nresid, array, Kname, &name);

  if (!name_equal(name, Kfunction))
    return ERROR_OK;

  CALL(arg_int, end, nresid, array, Kfunction, &i32);

  if (i32 != lfa->idx)
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
  err = foreach_document(env->e_end, env->e_resid, 1, ERROR_ELEMENT_NOT_FOUND,
			 lookup_function, &lfa);
  if (err != ERROR_FOUND)
    return err;

  return exec_arg(env, lfa.end, lfa.resid, 0, Kblocks);
}

static int
mode_value(ntype mode)
{
  if (name_equal(mode, KON))
    return 1;
  return 0;
}

static int
dcmode_value(ntype mode)
{
  if (name_equal(mode, KNORMAL))
    return 0;
  if (name_equal(mode, KREVERSE))
    return 1;
  if (name_equal(mode, KCOAST))
    return 2;
  if (name_equal(mode, KBRAKE))
    return 3;
  return 0;
}

static int
acceldir_value(ntype mode)
{
  if (name_equal(mode, Kx))
    return 1;
  if (name_equal(mode, Ky))
    return 2;
  if (name_equal(mode, Kz))
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
    { KV0, 0 },
    { KV1, 1 },

    { KV2, 2 },
    { KV3, 3 },
    { KV4, 6 },
    { KV5, 7 },
    { KV6, 8 },
    { KV7, 9 },
    { KV8, 11 },
    { KV9, 13 },

    { KK0, 0 },
    { KK1, 1 },

    { KK2, 24 },
    { KK3, 25 },
    { KK4, 26 },
    { KK5, 27 },
    { KK6, 28 },
    { KK7, 29 },

    { KA0, 24 },
    { KA1, 25 },
    { KA2, 26 },
    { KA3, 27 },
  };

  for (int i = 0; i < sizeof(pv) / sizeof(pv[0]); i++) {
    if (name_equal(pv[i].port, port))
      return pv[i].value;
  }
  return 0;
}

static int
setup_ss(env *env, const uint8_t *end, ssize_t *resid, int array,
	 struct servo_sync *ss)
{
  ntype port = 0;
  ntype name;
  uint32_t u32;
  ssize_t nresid;
  int err;

  CALL(narrow_to_elist, &end, resid, &nresid);
  CALL(arg_keyword, end, nresid, array, Kname, &name);

  if (!name_equal(name, Kset_servomotor_degree))
    return ERROR_INVALID_TYPE;

  CALL(arg_keyword, end, nresid, array, Kport, &port);
  CALL(exec_arg, env, end, nresid, array, Kdegree);

  ss->port = port_value(port);
  ss->degree = env->e_value;
  return ERROR_OK;
}

static int
init_servo_sync(env *env, const uint8_t *end, ssize_t resid, int array,
		struct servo_sync *ss, size_t *count)
{
  ssize_t nresid;
  const size_t max_count = *count;
  int err;

  *count = 0;
  CALL(elist_lookup, end, &resid, array, Kblocks);
  const int type = *(end - resid);
  if (type != BT_ARRAY)
    return ERROR_INVALID_TYPE;
  resid -= skip_name(end, resid, array);
  CALL(narrow_to_elist, &end, &resid, &nresid);
  for (;;) {
    if (nresid < ELIST_SIZE(0))
      return ERROR_OVERFLOW;
    if (nresid == ELIST_SIZE(0)) /* trailing nul */
      return ERROR_OK;
    const int type = *(end - nresid);
    if (type != BT_DOCUMENT)
      return ERROR_INVALID_TYPE;
    if (*count == max_count)
      return ERROR_TOOMANY_SERVO;
    nresid -= skip_name(end, nresid, 1);
    CALL(setup_ss, env, end, &nresid, 0, &ss[(*count)++]);
  }
}

static int
exec_block(env *env, const uint8_t *end, ssize_t *resid)
{
  const int array = 0;
  ssize_t nresid;
  ntype name;
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_block");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }

  CALL(narrow_to_elist, &end, resid, &nresid);
  CALL(arg_keyword, end, nresid, array, Kname, &name);

  switch (name) {
  case Kwhen_green_flag_clicked: {
    return exec_arg(env, end, nresid, array, Kblocks);
  }

  case Krepeat: {
    CALL(exec_arg, env, end, nresid, array, Kcount);
    ssize_t count = env->e_value;
    while (count-- > 0) {
      CHECK_INTR(ERROR_INTERRUPTED);
      CALL(exec_arg, env, end, nresid, array, Kblocks);
    }
    return ERROR_OK;
  }

  case Krepeat_until: {
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
      CALL(exec_arg, env, end, nresid, array, Kcondition);
      if (env->e_value != 0)
	break;
      CALL(exec_arg, env, end, nresid, array, Kblocks);
    }
    return ERROR_OK;
  }

  case Kwait_until: {
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
      CALL(exec_arg, env, end, nresid, array, Kcondition);
      if (env->e_value != 0)
	break;
      CALL(EX_DELAY, 0.02);
    }
    return ERROR_OK;
  }

  case Kforever: {
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
      CALL(exec_arg, env, end, nresid, array, Kblocks);
    }
    return ERROR_OK;
  }

  case Kif_then: {
    CALL(exec_arg, env, end, nresid, array, Kcondition);
    if (env->e_value != 0)
      CALL(exec_arg, env, end, nresid, array, Kblocks);
    return ERROR_OK;
  }

  case Kif_then_else: {
    CALL(exec_arg, env, end, nresid, array, Kcondition);
    if (env->e_value != 0)
      CALL(exec_arg, env, end, nresid, array, Kthen_blocks);
    else
      CALL(exec_arg, env, end, nresid, array, Kelse_blocks);
    return ERROR_OK;
  }

  case Kwait: {
    CALL(exec_arg, env, end, nresid, array, Ksecs);
    CALL(EX_DELAY, env->e_value);
    return ERROR_OK;
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

  case Kmath: {
    ntype op;

    CALL(arg_keyword, end, nresid, array, Kop, &op);
    CALL(exec_arg, env, end, nresid, array, Kx);
    switch (op) {
    case Kabs:
#if 1
      env->e_value = fabsf(env->e_value);
#else
      if (env->e_value < 0)
	env->e_value = -env->e_value;
#endif
      break;
    case Ksqrt:
      env->e_value = sqrtf(env->e_value);
      break;
    case Ksin:
      env->e_value = sinf(deg2rad(env->e_value));
      break;
    case Kcos:
      env->e_value = cosf(deg2rad(env->e_value));
      break;
    case Ktan:
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
    case Kln:
      env->e_value = logf(env->e_value);
      break;
    case Klog:
#if 0
      env->e_value = log10f(env->e_value);
#else
      env->e_value = logf(env->e_value) / M_LN10;
#endif
      break;
    case Kec:			/* e^ */
#if 0
      env->e_value = expf(env->e_value);
#else
      env->e_value = powf(M_E, env->e_value);
#endif
      break;
    case K10c:			/* 10^ */
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

  case Kturn_led: {
    ntype port = 0;
    ntype mode = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(arg_keyword, end, nresid, array, Kmode, &mode);
    EX_TURN_LED(port_value(port), mode_value(mode));
    return ERROR_OK;
  }

  case Kmulti_led: {
    CALL(exec_arg, env, end, nresid, array, Kr);
    vtype r = env->e_value;
    CALL(exec_arg, env, end, nresid, array, Kg);
    vtype g = env->e_value;
    CALL(exec_arg, env, end, nresid, array, Kb);
    vtype b = env->e_value;
    EX_MULTILED(r, g, b);
    return ERROR_OK;
  }

  case Kturn_dcmotor_on: {
    ntype port = 0;
    ntype direction = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(arg_keyword, end, nresid, array, Kdirection, &direction);
    EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(direction));
    return ERROR_OK;
  }

  case Kturn_dcmotor_off: {
    ntype port = 0;
    ntype mode = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(arg_keyword, end, nresid, array, Kmode, &mode);
    EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(mode));
    return ERROR_OK;
  }

  case Kset_dcmotor_power: {
    ntype port = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(exec_arg, env, end, nresid, array, Kpower);
    EX_SET_DCMOTOR_POWER(port_value(port), env->e_value);
    return ERROR_OK;
  }

  case Kbuzzer_on: {
    ntype port = 0;
    uint32_t u32;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(exec_arg, env, end, nresid, array, Kfrequency);
    EX_BUZZER_CONTROL(port_value(port), 1, env->e_value);
    return ERROR_OK;
  }

  case Kbuzzer_off: {
    ntype port = 0;
    uint32_t u32;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    EX_BUZZER_CONTROL(port_value(port), 0, 0);
    return ERROR_OK;
  }

  case Kset_servomotor_degree: {
    ntype port = 0;
    uint32_t u32;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(exec_arg, env, end, nresid, array, Kdegree);
    EX_SERVO_MOTOR(port_value(port), env->e_value);
    return ERROR_OK;
  }

  case Kservomotor_synchronized_motion: {
    CALL(exec_arg, env, end, nresid, array, Kspeed);
    const int time = env->e_value;
    struct servo_sync ss[MAX_SERVOS];
    size_t count = sizeof(ss) / sizeof(ss[0]);
    CALL(init_servo_sync, env, end, nresid, array, ss, &count);
    EX_SERVOMOTOR_SYNCHRONIZED_MOTION(ss, count, time);
    return ERROR_OK;
  }

  case Kir_photo_reflector_value:
  case Klight_sensor_value: {
    ntype port = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    env->e_value = EX_ANALOG_SENSOR(port_value(port));
    return ERROR_OK;
  }

  case K3_axis_digital_accelerometer_value: {
    ntype port = 0;
    ntype dir = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(arg_keyword, end, nresid, array, Kdirection, &dir);
    env->e_value = EX_ACCELEROMETER_VALUE(port_value(port),
					  acceldir_value(dir));
    return ERROR_OK;
  } 

  case Kbutton_value:
  case Ktouch_sensor_value: {
    ntype port = 0;
    ntype mode = 0;

    CALL(arg_keyword, end, nresid, array, Kport, &port);
    CALL(arg_keyword, end, nresid, array, Kmode, &mode);
    if (name_equal(mode, KON))
      env->e_value = EX_DIGITAL_SENSOR(port_value(port)) == 0;
    else
      env->e_value = EX_DIGITAL_SENSOR(port_value(port)) != 0;
    return ERROR_OK;
  }

  case Kvariable_ref: {
    uint32_t u32;

    CALL(lookup_variable, env, end, nresid, &u32);
    env->e_value = env->e_vars[u32];
    return ERROR_OK;
  }

  case Kset_variable_to:
  case Kchange_variable_by: {
    const int assign = name == Kset_variable_to;
    uint32_t u32;

    CALL(lookup_variable, env, end, nresid, &u32);
    CALL(exec_arg, env, end, nresid, array, Kvalue);
    if (assign)
      env->e_vars[u32] = env->e_value;
    else
      env->e_vars[u32] += env->e_value;
    return ERROR_OK;
  }

  case Kcall_function: {
    int32_t i32;

    CALL(arg_int, end, nresid, array, Kfunction, &i32);
    return exec_function(env, i32);
  }

  case Kpick_random: {
    CALL(exec_arg, env, end, nresid, array, Kfrom);
    const vtype from = env->e_value;
    CALL(exec_arg, env, end, nresid, array, Kto);
    const vtype to = env->e_value;
    env->e_value = EX_RANDOM(from, to + 1);
    return ERROR_OK;
  }

  case Kbreakpoint:
  case Kfunction:
  case Kvariable:
  case Klist:
    return ERROR_OK;		/* NOP */

  case Kreset_timer: {
    EX_RESET_TIMER();
    return ERROR_OK;
  }
  case Ktimer: {
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
exec(env *env, const uint8_t *end, ssize_t *resid, int array)
{
  const uint8_t *p = end - *resid;
  uint32_t u32;
  int err;
  union {
    double d;
    float f;
    int8_t i8;
    int16_t i16;
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
    *resid -= skip_name(end, *resid, array);
    return exec_block(env, end, resid);
  case BT_ARRAY:
    *resid -= skip_name(end, *resid, array);
    return exec_array(env, end, resid);
  case BT_NUMBER:
    *resid -= skip_name(end, *resid, array);
    for (int i = 0; i < 4; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.f;		/* convert from float to vtype */
    *resid -= 4;
    return ERROR_OK;
  case BT_INT8:
    *resid -= skip_name(end, *resid, array);
    for (int i = 0; i < 1; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.i8;	/* convert from 16 bit integer to vtype */
    *resid -= 1;
    return ERROR_OK;
  case BT_INT16:
    *resid -= skip_name(end, *resid, array);
    for (int i = 0; i < 2; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.i16;	/* convert from 16 bit integer to vtype */
    *resid -= 2;
    return ERROR_OK;
  case BT_INT32:
    *resid -= skip_name(end, *resid, array);
    for (int i = 0; i < 4; i++)
      u.b[i] = *(end - *resid + i);
    env->e_value = u.i32;	/* convert from 32 bit integer to vtype */
    *resid -= 4;
    return ERROR_OK;
  case BT_INT64:
    *resid -= skip_name(end, *resid, array);
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
exec_array(env *env, const uint8_t *end, ssize_t *resid)
{
  ssize_t nresid;
  int err;

  if ((void *)&err < env->e_stack) {
    EX_TRACE("exec_array");
    EX_TRACE_HEX((int)&err);
    env->e_stack = &err;
  }
  CALL(narrow_to_elist, &end, resid, &nresid);

  for (;;) {
    if (nresid < ELIST_SIZE(0))
      return ERROR_OVERFLOW;
    if (nresid == ELIST_SIZE(0)) /* trailing nul */
      return ERROR_OK;
    CALL(exec, env, end, &nresid, 1);
  }
}

static int
port_init(const uint8_t *end, ssize_t *resid)
{
  const uint8_t *p = end - *resid;
  int err;

  if (resid <= 0)
    return ERROR_OVERFLOW;
  if (*p != BT_KEYWORD)
    return ERROR_INVALID_TYPE;
  ntype port = name_at(p + 1);
  *resid -= skip_name(end, *resid, 0);
  ntype part = name_at(end - *resid);
  *resid -= 2;
  CALL(EX_PORT_INIT, port_value(port), part);
  return ERROR_OK;
}

static int
setup_ports(const uint8_t *end, ssize_t resid)
{
  ssize_t nresid;
  int err;

  err = elist_lookup(end, &resid, 0, Kport_settings);
  switch (err) {
  case ERROR_OK: {
    const int type = *(end - resid);
    if (type != BT_DOCUMENT)
      return ERROR_INVALID_TYPE;
    resid -= skip_name(end, resid, 0);
    CALL(narrow_to_elist, &end, &resid, &nresid);
    for (;;) {
      if (nresid < ELIST_SIZE(0))
	return ERROR_OVERFLOW;
      if (nresid == ELIST_SIZE(0))		/* trailing nul */
	return ERROR_OK;
      CALL(port_init, end, &nresid);
    }
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
parse_fvl(const uint8_t *end, ssize_t *resid, int array, void *arg)
{
  parse_fvl_args *pfa = (parse_fvl_args *)arg;
  ssize_t nresid;
  ntype name;
  int err;

  CALL(narrow_to_elist, &end, resid, &nresid);
  CALL(arg_keyword, end, nresid, array, Kname, &name);

  switch (name) {
  case Kvariable:
    (*pfa->n_vars)++;
    return ERROR_OK;
  case Klist:
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
  return foreach_document(end, resid, 1, ERROR_OK, parse_fvl, &pfa);
}

static int
exec_script(const uint8_t *end, ssize_t *resid)
{
  env e_store, *env = &e_store;
  int err, n_vars, n_lsts;

  EX_TRACE_HEX((int)&err);
  CALL(setup_ports, end, *resid);

  CALL(elist_lookup, end, resid, 0, Kscripts);
  const int type = *(end - *resid);
  if (type != BT_ARRAY)
    return ERROR_UNSUPPORTED;

  //EX_TRACE("script found");
  /*
   * At this point, we're looking at:
   *   "\x04" "scripts\x00" document
   *      ||
   *   "\x04" "scripts\x00" int32 e_list "\x00" 
   */
  *resid -= skip_name(end, *resid, 0);

  /* Now, we're looking at (e_list is an array of blocks):
   *   int32 e_list "\x00" 
   */

  CALL(count_environ, end, *resid, &n_vars, &n_lsts);
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
  CALL(exec_array, env, end, resid);
  return ERROR_OK;
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
  CALL(read32, end, resid, &u32);
  if (u32 != size) {
    //EX_TRACE("interp: size mismatch");
    //EX_TRACE_INT(u32);
    //EX_TRACE_INT(size);
    return ERROR_BUFFER_TOO_SHORT;
  }
#if !defined(SIZE16)
  const size_t ssize = 4;
#else
  const size_t ssize = 2;
#endif
  if (size < ELIST_SIZE(ssize)) {
    //EX_TRACE("interp: size too small");
    return ERROR_BUFFER_TOO_SHORT;
  }

  end -= ELIST_SIZE(0);		/* drop trailing 0 */
  resid -= ELIST_SIZE(ssize);	/* leading int32 + trailing 0 */
  return exec_script(end, &resid);
}
