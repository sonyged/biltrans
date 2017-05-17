/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

DEFUN(when_green_flag_clicked, {
  return exec_blocks(ctx, Kblocks);
})

DEFUN(repeat, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Kcount);
  ssize_t count = env->e_value;
  while (count-- > 0) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_blocks, ctx, Kblocks);
  }
  return ERROR_OK;
})

DEFUN(forever, {

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_blocks, ctx, Kblocks);
  }
  return ERROR_OK;
})

DEFUN(if_then, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Kcondition);
  if (env->e_value != 0)
    RETURN(exec_blocks, ctx, Kblocks);
  return ERROR_OK;
})

DEFUN(if_then_else, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Kcondition);
  if (env->e_value != 0)
    RETURN(exec_blocks, ctx, Kthen_blocks);
  else
    RETURN(exec_blocks, ctx, Kelse_blocks);
  return ERROR_OK;
})

DEFUN(wait, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Ksecs);
  CALL(EX_DELAY, env->e_value);
  return ERROR_OK;
})

DEFUN(wait_until, {
  env *env = ctx->c_env;

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_arg, ctx, Kcondition);
    if (env->e_value != 0)
      break;
    CALL(EX_DELAY, 0.02);
  }
  return ERROR_OK;
})

DEFUN(repeat_until, {
  env *env = ctx->c_env;

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_arg, ctx, Kcondition);
    if (env->e_value != 0)
      break;
    CALL(exec_blocks, ctx, Kblocks);
  }
  return ERROR_OK;
})

DEFUN(breakpoint, {

  return ERROR_OK;		/* NOP */
})

EXEC_BINARY(plus)
EXEC_BINARY(minus)
EXEC_BINARY(multiply)
EXEC_BINARY(divide)
EXEC_BINARY(mod)

EXEC_BINARY(and)
EXEC_BINARY(or)
EXEC_UNARY(not)

EXEC_BINARY(equal)
EXEC_BINARY(less_than)
EXEC_BINARY(greater_than)
EXEC_BINARY(less_than_or_equal)
EXEC_BINARY(greater_than_or_equal)

EXEC_UNARY(round)

DEFUN(math, {
  env *env = ctx->c_env;
  ntype op;

  CALL(arg_keyword, &ctx->c_region, Kop, &op);
  CALL(exec_arg, ctx, Kx);
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
})

DEFUN(turn_led, {
  ntype port = 0;
  ntype mode = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(arg_keyword, &ctx->c_region, Kmode, &mode);
  EX_TURN_LED(port_value(port), mode_value(mode));
  return ERROR_OK;
})

DEFUN(turn_dcmotor_on, {
  ntype port = 0;
  ntype direction = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(arg_keyword, &ctx->c_region, Kdirection, &direction);
  EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(direction));
  return ERROR_OK;
})

DEFUN(turn_dcmotor_off, {
  ntype port = 0;
  ntype mode = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(arg_keyword, &ctx->c_region, Kmode, &mode);
  EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(mode));
  return ERROR_OK;
})

DEFUN(buzzer_on, {
  env *env = ctx->c_env;
  ntype port = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(exec_arg, ctx, Kfrequency);
  EX_BUZZER_CONTROL(port_value(port), 1, env->e_value);
  return ERROR_OK;
})

DEFUN(buzzer_off, {
  ntype port = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  EX_BUZZER_CONTROL(port_value(port), 0, 0);
  return ERROR_OK;
})

DEFUN(set_servomotor_degree, {
  env *env = ctx->c_env;
  ntype port = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(exec_arg, ctx, Kdegree);
  EX_SERVO_MOTOR(port_value(port), env->e_value);
  return ERROR_OK;
})

DEFUN(set_dcmotor_power, {
  env *env = ctx->c_env;
  ntype port = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(exec_arg, ctx, Kpower);
  EX_SET_DCMOTOR_POWER(port_value(port), env->e_value);
  return ERROR_OK;
})

DEFUN(multi_led, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Kr);
  vtype r = env->e_value;
  CALL(exec_arg, ctx, Kg);
  vtype g = env->e_value;
  CALL(exec_arg, ctx, Kb);
  vtype b = env->e_value;
  EX_MULTILED(r, g, b);
  return ERROR_OK;
})

DEFUN(servomotor_synchronized_motion, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Kspeed);
  const int time = env->e_value;
  struct servo_sync ss[MAX_SERVOS];
  size_t count = sizeof(ss) / sizeof(ss[0]);
  CALL(init_servo_sync, ctx, ss, &count);
  EX_SERVOMOTOR_SYNCHRONIZED_MOTION(ss, count, time);
  return ERROR_OK;
})

DEFUN(3_axis_digital_accelerometer_value, {
  env *env = ctx->c_env;
  ntype port = 0;
  ntype dir = 0;

  CALL(arg_keyword, &ctx->c_region, Kport, &port);
  CALL(arg_keyword, &ctx->c_region, Kdirection, &dir);
  env->e_value = EX_ACCELEROMETER_VALUE(port_value(port),
					acceldir_value(dir));
  return ERROR_OK;
})

DEFUN(ir_photo_reflector_value, {

  return analog_sensor_value(ctx, 1023);
})

DEFUN(light_sensor_value, {

  return analog_sensor_value(ctx, 1023);
})

DEFUN(button_value, {

  return digital_sensor_value(ctx);
})

DEFUN(touch_sensor_value, {

  return digital_sensor_value(ctx);
})

DEFUN(reset_timer, {

  EX_RESET_TIMER();
  return ERROR_OK;
})

DEFUN(timer, {
  env *env = ctx->c_env;

  env->e_value = EX_TIMER();
  return ERROR_OK;
})

DEFUN(pick_random, {
  env *env = ctx->c_env;

  CALL(exec_arg, ctx, Kfrom);
  const vtype from = env->e_value;
  CALL(exec_arg, ctx, Kto);
  const vtype to = env->e_value;
  env->e_value = EX_RANDOM(from, to + 1);
  return ERROR_OK;
})

DEFUN(function, {

  return ERROR_OK;		/* NOP */
})

DEFUN(call_function, {
  env *env = ctx->c_env;
  int32_t i32;

  CALL(arg_int, &ctx->c_region, Kfunction, &i32);
  return exec_function(env, i32);
})

DEFUN(variable, {

  return ERROR_OK;		/* NOP */
})

DEFUN(variable_ref, {
  env *env = ctx->c_env;
  uint32_t u32;

  CALL(lookup_variable, ctx, &u32);
  env->e_value = env->e_vars[u32];
  return ERROR_OK;
})

DEFUN(set_variable_to, {
  env *env = ctx->c_env;
  uint32_t u32;

  CALL(lookup_variable, ctx, &u32);
  CALL(exec_arg, ctx, Kvalue);
  env->e_vars[u32] = env->e_value;
  return ERROR_OK;
})

DEFUN(change_variable_by, {
  env *env = ctx->c_env;
  uint32_t u32;

  CALL(lookup_variable, ctx, &u32);
  CALL(exec_arg, ctx, Kvalue);
  env->e_vars[u32] += env->e_value;
  return ERROR_OK;
})

DEFUN(list, {

  return ERROR_OK;		/* NOP */
})

DEFUN(list_length, {
  env *env = ctx->c_env;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  env->e_value = list_length(&env->e_lsts[u32]);
  return ERROR_OK;
})

DEFUN(list_add, {
  env *env = ctx->c_env;
  int err;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  CALL(exec_arg, ctx, Kvalue);
  list_add(&env->e_lsts[u32], env->e_value, &err);
  return list_error(err);
})

DEFUN(list_contains, {
  env *env = ctx->c_env;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  CALL(exec_arg, ctx, Kvalue);
  env->e_value = list_contains(&env->e_lsts[u32], env->e_value);
  return ERROR_OK;
})

DEFUN(list_ref, {
  env *env = ctx->c_env;
  int err;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  CALL(exec_arg, ctx, Kposition);
  const vtype position = LIST_POSITION(env->e_value);
  env->e_value = list_ref(&env->e_lsts[u32], position, &err);
  return list_error(err);
})

DEFUN(list_delete, {
  env *env = ctx->c_env;
  int err;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  CALL(exec_arg, ctx, Kposition);
  const vtype position = LIST_POSITION(env->e_value);
  list_delete(&env->e_lsts[u32], position, &err);
  return list_error(err);
})

DEFUN(list_replace, {
  env *env = ctx->c_env;
  int err;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  CALL(exec_arg, ctx, Kvalue);
  const vtype value = env->e_value;
  CALL(exec_arg, ctx, Kposition);
  const vtype position = LIST_POSITION(env->e_value);
  list_replace(&env->e_lsts[u32], position, value, &err);
  return list_error(err);
})

DEFUN(list_insert, {
  env *env = ctx->c_env;
  int err;
  uint32_t u32;

  CALL(lookup_list, ctx, &u32);
  CALL(exec_arg, ctx, Kvalue);
  const vtype value = env->e_value;
  CALL(exec_arg, ctx, Kposition);
  const vtype position = LIST_POSITION(env->e_value);
  list_insert(&env->e_lsts[u32], position, value, &err);
  return list_error(err);
})

DEFUN(sound_sensor_value, {

  return analog_sensor_value(ctx, 1023 * (3.3 - 1.5) / 3.3);
})
