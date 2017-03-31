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
#if !defined(INSN_DICT_H)
#include "insn_dict.h"
#endif
#if !defined(LISTLIB_H)
#include "listlib.h"
#endif

typedef float vtype;

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

typedef struct region {
  const uint8_t *r_end;
  ssize_t r_resid;
} region;

typedef struct ctx {
  env *c_env;
  region c_region;
#define c_end c_region.r_end
#define c_resid c_region.r_resid
} ctx;

static int arg_keyword(const region *, ntype, ntype *);
static int arg_int(const region *, ntype, int32_t *);

#undef ELIST_NUL
#if defined(ELIST_NUL)
#define ELIST_SIZE(s)	((s) + 1) /* elist with trailing NUL */
#else
#define ELIST_SIZE(s)	((s) + 0)
#endif

static int
read_size(const uint8_t *end, const ssize_t resid, uint32_t *v)
{
  const uint8_t *p = end - resid;

#define SIZE16
#if !defined(SIZE16)
#define SIZE_SIZE 4
  if (resid < sizeof(uint32_t))
    return ERROR_BUFFER_TOO_SHORT;
  *v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
#else
#define SIZE_SIZE 2
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

static int
get_type(const uint8_t *end, ssize_t *resid, int array)
{

  if (*resid <= 0)
    return BT_ERROR;

  const uint8_t type = *(end - *resid);
  *resid -= skip_name(end, *resid, array);
  if (*resid <= 0)
    return BT_ERROR;
  return type;
}

#define LOG(fmt, ...) do {} while (0)
//#define LOG(fmt, ...) do { fprintf(stderr, fmt, __VA_ARGS__); } while (0)
//#define LOG(fmt, ...) do { EX_TRACE(fmt); } while (0)

#define CALL(F, ...) do {			\
  int err = (F)(__VA_ARGS__);			\
  if (err) {					\
    LOG(#F ": %d: err = %d\n", __LINE__, err);	\
    return err;					\
  }						\
} while (0)

#if 0
#define RETURN(F, ...) do {			\
  int err = (F)(__VA_ARGS__);			\
  if (err) {					\
    LOG(#F ": %d: err = %d\n", __LINE__, err);	\
    return err;					\
  }						\
} while (0)
#else
#define RETURN(F, ...) do {			\
  return (F)(__VA_ARGS__);			\
} while (0)
#endif

#define CHECK_STACK0(env, where, p, q) do {	\
  if ((void *)(p) < env->e_stack) {		\
    EX_TRACE(where);				\
    EX_TRACE_HEX((int)(p));			\
    EX_TRACE_HEX((int)(q));			\
    env->e_stack = (p);				\
  }						\
} while (0)

#define CHECK_STACK(env, where, p) do {		\
  if ((void *)(p) < env->e_stack) {		\
    EX_TRACE(where);				\
    EX_TRACE_HEX((int)(p));			\
    env->e_stack = (p);				\
  }						\
} while (0)


/*
 * Narrow the region down to current elist.
 */
static int
narrow_to_elist(const uint8_t **end, ssize_t *resid, ssize_t *nresid)
{
  uint32_t u32;
  const size_t size = SIZE_SIZE;
  int err;

  CALL(read_size, *end, *resid, &u32);
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
    uint32_t u32;
    int err;

    if ((*compare)(end, r, arg)) {
      *resid = r;
      return ERROR_OK;
    }

    const uint8_t type = get_type(end, &r, array);
    switch (type) {
    case BT_ERROR:
      return ERROR_BUFFER_TOO_SHORT;
    case BT_NUMBER:
      r -= 4;
      break;
    case BT_KEYWORD:
      r -= 2;
      break;
    case BT_OBJECT:
    case BT_ARRAY:
      CALL(read_size, end, r, &u32);
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

  return elist_find(end, resid, 0, compare_name, (void *)&name);
}

static int
compare_keyword(const region *region, int array, ntype name, ntype value)
{
  ntype p = 0;

  return arg_keyword(region, name, &p) == ERROR_OK &&
    name_equal(p, value);
}

typedef struct names {
  ntype n_name;
  ntype n_value;
} names;

static int
compare_names(const region *region, int array, size_t n, const names *names)
{
  const uint8_t *end = region->r_end;
  ssize_t resid = region->r_resid;
  ssize_t nresid;
  const uint8_t type = get_type(end, &resid, array);

  if (type != BT_OBJECT)
    return 0;
  if (narrow_to_elist(&end, &resid, &nresid) != ERROR_OK)
    return 0;

  struct region nregion;
  nregion.r_end = end;
  nregion.r_resid = nresid;
  for (size_t i = 0; i < n; i++)
    if (!compare_keyword(&nregion, 0, names[i].n_name, names[i].n_value))
      return 0;
  return 1;
}

static int
compare_function(const region *region, int array, void *arg)
{
  ntype name = *(ntype *)arg;
  names names[] = {
    { Kname, Ifunction },
    { Kfunction, name },
  };

  return compare_names(region, array, 2, names);
}

static int exec_array(env *env, const uint8_t *end, ssize_t *resid);
static int exec_block(env *env, const uint8_t *end, ssize_t *resid);
static int exec_number(env *env, const uint8_t *end, ssize_t *resid,
		       uint8_t type);

static int
arg_keyword(const region *region, ntype name, ntype *v)
{
  int err;
  const uint8_t *end = region->r_end;
  ssize_t resid = region->r_resid;

  CALL(elist_lookup, end, &resid, name);
  const uint8_t type = get_type(end, &resid, 0);
  if (type != BT_KEYWORD)
    return ERROR_INVALID_TYPE;
  if (resid <= 0)
    return ERROR_BUFFER_TOO_SHORT;
  *v = name_at(end - resid);
  //printf("arg_keyword: %s = %s\n", name, *v);
  return ERROR_OK;
}

static int
arg_int(const region *region, ntype name, int32_t *i32)
{
  const uint8_t *end = region->r_end;
  ssize_t resid = region->r_resid;
  int err;

  CALL(elist_lookup, end, &resid, name);
  const uint8_t type = get_type(end, &resid, 0);
  const uint8_t *q = end - resid;
  switch (type) {
  case BT_ERROR:
    return ERROR_BUFFER_TOO_SHORT;
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
exec_arg(const ctx *ctx, ntype name)
{
  int err;
  env *env = ctx->c_env;
  const uint8_t *end = ctx->c_end;
  ssize_t resid = ctx->c_resid;

  CHECK_STACK0(env, "exec_arg", &err, name);
  CALL(elist_lookup, end, &resid, name);

  const uint8_t type = get_type(end, &resid, 0);
  switch (type) {
  case BT_ERROR:
    return ERROR_BUFFER_TOO_SHORT;
  case BT_OBJECT:
    return exec_block(env, end, &resid);
  case BT_NUMBER:
  case BT_INT8:
  case BT_INT16:
  case BT_INT32:
    return exec_number(env, end, &resid, type);
  default:
    return ERROR_INVALID_TYPE;
  }
}

/*
 * Similar to exec_arg(), but we know an argument is blocks.  Calling
 * exec_arg() for this case consumes more stack.
 */
static int
exec_blocks(const ctx *ctx, ntype name)
{
  int err;
  const uint8_t *end = ctx->c_end;
  ssize_t resid = ctx->c_resid;

  CHECK_STACK0(ctx->c_env, "exec_blocks", &err, name);
  CALL(elist_lookup, end, &resid, name);
  const uint8_t type = get_type(end, &resid, 0);
  if (type != BT_ARRAY)
    return ERROR_INVALID_TYPE;
  return exec_array(ctx->c_env, end, &resid);
}

static int
exec_unary(const ctx *ctx, vtype (*f)(vtype x))
{
  env *env = ctx->c_env;
  int err;

  CHECK_STACK(env, "exec_unary", &err);
  CALL(exec_arg, ctx, Kx);
  env->e_value = (*f)(env->e_value);
  return ERROR_OK;
}

static int
exec_binary(const ctx *ctx, vtype (*f)(vtype x, vtype y))
{
  env *env = ctx->c_env;
  int err;

  CHECK_STACK(env, "exec_binary", &err);
  CALL(exec_arg, ctx, Kx);
  vtype x = env->e_value;
  CALL(exec_arg, ctx, Ky);
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
DEFBINARY(less_than_or_equal, <=);
DEFBINARY(greater_than_or_equal, >=);

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
lookup_index(const region *region, uint32_t *u32, ntype name, uint32_t limit)
{
  int err;
  int32_t i32;

  CALL(arg_int, region, name, &i32);
  if (i32 < 0 || i32 >= limit)
    return ERROR_OUT_OF_RANGE;
  *u32 = i32;
  return ERROR_OK;
}

static int
lookup_variable(const ctx *ctx, uint32_t *u32)
{
  env *env = ctx->c_env;

  return lookup_index(&ctx->c_region, u32, Kvariable, env->e_nvars);
}

static int
lookup_list(const ctx *ctx, uint32_t *u32)
{
  env *env = ctx->c_env;

  return lookup_index(&ctx->c_region, u32, Klist, env->e_nlsts);
}

static int
with_elist(const uint8_t *end, ssize_t *resid, int array,
	   int (*proc)(const region *region, int, void *), void *arg)
{
  ssize_t nresid;
  uint32_t u32;

  CALL(narrow_to_elist, &end, resid, &nresid);

  region nregion;
  nregion.r_end = end;
  nregion.r_resid = nresid;
  return (*proc)(&nregion, array, arg);
}

static int
foreach_document(const region *region, int array,
		 int end_of_document,
		 int (*proc)(struct region *region, int array, void *),
		 void *arg)
{
  const uint8_t *end = region->r_end;
  ssize_t resid = region->r_resid;
  ssize_t nresid;
  int err, type;

  CALL(narrow_to_elist, &end, &resid, &nresid);

  struct region nregion;
  nregion.r_end = end;
  nregion.r_resid = nresid;
  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    if (nregion.r_resid < ELIST_SIZE(0))
      return ERROR_OVERFLOW;
    if (nregion.r_resid == ELIST_SIZE(0)) /* trailing nul */
      return end_of_document;
    const uint8_t type = get_type(end, &nregion.r_resid, array);
    if (type != BT_OBJECT)
      return ERROR_INVALID_TYPE;
    CALL((*proc), &nregion, 0, arg);
  }
}

typedef struct lookup_function_args {
  uint32_t idx;
  struct ctx ctx;
} lookup_function_args;

static int
lookup_function(region *region, int array, void *arg)
{
  lookup_function_args *lfa = (lookup_function_args *)arg;
  const uint8_t *end = region->r_end;
  ntype name;
  int32_t i32;
  ssize_t nresid;
  int err;

  CALL(narrow_to_elist, &end, &region->r_resid, &nresid);

  struct region nregion;
  nregion.r_end = end;
  nregion.r_resid = nresid;
  CALL(arg_keyword, &nregion, Kname, &name);
  if (!name_equal(name, Ifunction))
    return ERROR_OK;

  CALL(arg_int, &nregion, Kfunction, &i32);
  if (i32 != lfa->idx)
    return ERROR_OK;

  lfa->ctx.c_end = end;
  lfa->ctx.c_resid = nresid;
  return ERROR_FOUND;
}

static int
exec_function(env *env, uint32_t idx)
{
  lookup_function_args lfa;
  int err;

  CHECK_STACK(env, "exec_function", &err);
  lfa.idx = idx;
  lfa.ctx.c_env = env;
  lfa.ctx.c_end = env->e_end;
  lfa.ctx.c_resid = env->e_resid;
  err = foreach_document(&lfa.ctx.c_region, 1, ERROR_ELEMENT_NOT_FOUND,
			 lookup_function, &lfa);
  if (err != ERROR_FOUND)
    return err;

  return exec_blocks(&lfa.ctx, Kblocks);
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
setup_ss(ctx *ctx, struct servo_sync *ss)
{
  env *env = ctx->c_env;
  const uint8_t *end = ctx->c_end;
  ntype port = 0;
  ntype name;
  uint32_t u32;
  ssize_t nresid;
  int err;

  CALL(narrow_to_elist, &end, &ctx->c_resid, &nresid);

  struct ctx nctx;
  nctx.c_env = env;
  nctx.c_end = end;
  nctx.c_resid = nresid;
  CALL(arg_keyword, &nctx.c_region, Kname, &name);
  if (!name_equal(name, Iset_servomotor_degree))
    return ERROR_INVALID_TYPE;

  CALL(arg_keyword, &nctx.c_region, Kport, &port);
  CALL(exec_arg, &nctx, Kdegree);

  ss->port = port_value(port);
  ss->degree = env->e_value;
  return ERROR_OK;
}

static int
init_servo_sync(const ctx *ctx,
		struct servo_sync *ss, size_t *count)
{
  env *env = ctx->c_env;
  const uint8_t *end = ctx->c_end;
  ssize_t resid = ctx->c_resid;
  ssize_t nresid;
  const size_t max_count = *count;
  int err;

  *count = 0;
  CALL(elist_lookup, end, &resid, Kblocks);
  uint8_t type = get_type(end, &resid, 0);
  if (type != BT_ARRAY)
    return ERROR_INVALID_TYPE;
  CALL(narrow_to_elist, &end, &resid, &nresid);
  struct ctx nctx;
  nctx.c_env = env;
  nctx.c_end = end;
  nctx.c_resid = nresid;
  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    if (nctx.c_resid < ELIST_SIZE(0))
      return ERROR_OVERFLOW;
    if (nctx.c_resid == ELIST_SIZE(0)) /* trailing nul */
      return ERROR_OK;
    type = get_type(end, &nctx.c_resid, 1);
    if (type != BT_OBJECT)
      return ERROR_INVALID_TYPE;
    if (*count == max_count)
      return ERROR_TOOMANY_SERVO;
    CALL(setup_ss, &nctx, &ss[(*count)++]);
  }
}

static int
list_error(int err)
{
  switch (err) {
  case LE_OK: return ERROR_OK;
  case LE_NO_MEMORY: return ERROR_NO_MEMORY;
  case LE_RANGE: return ERROR_OUT_OF_RANGE;
  }
  return ERROR_UNSUPPORTED;
}

static int
analog_sensor_value(const ctx *ctx)
{
  env *env = ctx->c_env;
  int err;
  ntype port = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  env->e_value = EX_ANALOG_SENSOR(port_value(port));
  return ERROR_OK;
}

static int
digital_sensor_value(const ctx *ctx)
{
  env *env = ctx->c_env;
  int err;
  ntype port = 0;
  ntype mode = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(arg_keyword, &ctx->c_region, Kmode, &mode);
  if (name_equal(mode, KON))
    env->e_value = EX_DIGITAL_SENSOR(port_value(port)) == 0;
  else
    env->e_value = EX_DIGITAL_SENSOR(port_value(port)) != 0;
  return ERROR_OK;
}

#define DISPATCH_TABLE
#if defined(DISPATCH_TABLE)
#undef EXEC_BINARY
#define EXEC_BINARY(name)					\
  static int							\
  F ## name(const ctx *ctx)					\
  {								\
    								\
    return exec_binary(ctx, f_ ## name);			\
  }
#undef EXEC_UNARY
#define EXEC_UNARY(name)					\
  static int							\
  F ## name(const ctx *ctx)					\
  {								\
    								\
    return exec_unary(ctx, f_ ## name);				\
  }
#undef DEFUN
#define DEFUN(sym, body)					\
static int							\
F ## sym(const ctx *ctx) body
#include "interp_insns.h"
#endif	/* DISPATCH_TABLE */

static int
exec_block(env *env, const uint8_t *end, ssize_t *resid)
{
  ssize_t nresid;
  ntype name;
  int err;

  CHECK_STACK(env, "exec_block", &err);
  CALL(narrow_to_elist, &end, resid, &nresid);

  ctx ctx;
  ctx.c_env = env;
  ctx.c_end = end;
  ctx.c_resid = nresid;
  CALL(arg_keyword, &ctx.c_region, Kname, &name);

#if !defined(DISPATCH_TABLE)

#undef EXEC_BINARY
#define EXEC_BINARY(name)					\
  case I ## name: {						\
    return exec_binary(ctx, f_ ## name);			\
  }
#undef EXEC_UNARY
#define EXEC_UNARY(name)					\
  case I ## name: {						\
    return exec_unary(ctx, f_ ## name);				\
  }

  switch (name) {
#undef DEFUN
#define DEFUN(sym, body) case I ## sym: body
#include "interp_insns.h"
  default:
    return ERROR_UNSUPPORTED;
  }

#else  /* DISPATCH_TABLE */

  static int (*const ops[])(const struct ctx *ctx) = {
    Fbreakpoint,		/* nop */
#undef DEFUN
#define DEFUN(sym, body) F ## sym,
#undef EXEC_BINARY
#define EXEC_BINARY(sym) F ## sym,
#undef EXEC_UNARY
#define EXEC_UNARY(sym) F ## sym,
#include "interp_insns.h"
  };

  if (name >= sizeof(ops) / sizeof(ops[0]))
    return ERROR_UNSUPPORTED;
  return ops[name](&ctx);

#endif	/* DISPATCH_TABLE */
}

static int
exec_number(env *env, const uint8_t *end, ssize_t *resid, uint8_t type)
{
  union {
    float f;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    uint8_t b[4];
  } u;
  size_t size;

  switch (type) {
  case BT_NUMBER:
  case BT_INT32:
    size = 4;
    break;
  case BT_INT8:
    size = 1;
    break;
  case BT_INT16:
    size = 2;
    break;
  }

  if (*resid < size)
    return ERROR_INVALID_SIZE;
  for (int i = 0; i < size; i++)
    u.b[i] = *(end - *resid + i);
  *resid -= size;

  switch (type) {
  case BT_NUMBER:
    env->e_value = u.f;		/* convert from float to vtype */
    break;
  case BT_INT8:
    env->e_value = u.i8;	/* convert from 8 bit integer to vtype */
    break;
  case BT_INT16:
    env->e_value = u.i16;	/* convert from 16 bit integer to vtype */
    break;
  case BT_INT32:
    env->e_value = u.i32;	/* convert from 32 bit integer to vtype */
    break;
  }

  return ERROR_OK;
}

static int
exec_array(env *env, const uint8_t *end, ssize_t *resid)
{
  ssize_t nresid;
  int err;

  CHECK_STACK(env, "exec_array", &err);
  CALL(narrow_to_elist, &end, resid, &nresid);

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    if (nresid < ELIST_SIZE(0))
      return ERROR_OVERFLOW;
    if (nresid == ELIST_SIZE(0)) /* trailing nul */
      return ERROR_OK;
    const uint8_t type = get_type(end, &nresid, 1);
    if (type != BT_OBJECT)
      return ERROR_INVALID_TYPE;
    CALL(exec_block, env, end, &nresid);
  }
}

static int
port_init(const uint8_t *end, ssize_t *resid)
{
  const uint8_t *p = end - *resid;
  int err;

  const uint8_t type = get_type(end, resid, 0);
  if (type != BT_KEYWORD)
    return ERROR_INVALID_TYPE;
  ntype port = name_at(p + 1);
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

  err = elist_lookup(end, &resid, Kport_settings);
  switch (err) {
  case ERROR_OK: {
    const uint8_t type = get_type(end, &resid, 0);
    if (type != BT_OBJECT)
      return ERROR_INVALID_TYPE;
    CALL(narrow_to_elist, &end, &resid, &nresid);
    for (;;) {
      CHECK_INTR(ERROR_INTERRUPTED);
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
parse_fvl(region *region, int array, void *arg)
{
  parse_fvl_args *pfa = (parse_fvl_args *)arg;
  const uint8_t *end = region->r_end;
  ssize_t nresid;
  ntype name;
  int err;

  CALL(narrow_to_elist, &end, &region->r_resid, &nresid);

  struct region nregion;
  nregion.r_end = end;
  nregion.r_resid = nresid;
  CALL(arg_keyword, &nregion, Kname, &name);

  switch (name) {
  case Ivariable:
    (*pfa->n_vars)++;
    return ERROR_OK;
  case Ilist:
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
  region region;
  region.r_end = end;
  region.r_resid = resid;
  return foreach_document(&region, 1, ERROR_OK, parse_fvl, &pfa);
}

static int
exec_script(const uint8_t *end, ssize_t *resid)
{
  env e_store, *env = &e_store;
  int err, n_vars, n_lsts;

  EX_TRACE_HEX((int)&err);
  CALL(setup_ports, end, *resid);

  CALL(elist_lookup, end, resid, Kscripts);

  const uint8_t type = get_type(end, resid, 0);
  if (type != BT_ARRAY)
    return ERROR_INVALID_TYPE;

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
  ssize_t resid = SIZE_SIZE;
  const uint8_t *end = p + resid;
  uint32_t u32;

  /*
   * int32 e_list "\x00"
   */
  CALL(read_size, end, resid, &u32);
#if defined(SIZE16)
  if (u32 > 0xffff)
    return ERROR_INVALID_SIZE;
#endif
#if defined(KOOV_MAGIC)
  const uint8_t *q = p + u32;
  if (q[0] != (KOOV_MAGIC & 0xff) ||
      q[1] != ((KOOV_MAGIC >> 8) & 0xff) ||
      q[2] != ((KOOV_MAGIC >> 16) & 0xff) ||
      q[3] != ((KOOV_MAGIC >> 24) & 0xff))
    return ERROR_INVALID_MAGIC;
#endif

  end = p + u32;
  resid = u32;

  if (resid < ELIST_SIZE(SIZE_SIZE)) {
    //EX_TRACE("interp: size too small");
    return ERROR_BUFFER_TOO_SHORT;
  }

  end -= ELIST_SIZE(0);		/* drop trailing 0 if any */
  resid -= ELIST_SIZE(SIZE_SIZE); /* leading int32 + trailing 0 */
  return exec_script(end, &resid);
}
