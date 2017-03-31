DEFUN(when_green_flag_clicked, {
 return exec_blocks(env, end, nresid, Kblocks);
})

DEFUN(repeat, {
  int err;

  CALL(exec_arg, env, end, nresid, Kcount);
  ssize_t count = env->e_value;
  while (count-- > 0) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_blocks, env, end, nresid, Kblocks);
  }
  return ERROR_OK;
})

DEFUN(forever, {
  int err;

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_blocks, env, end, nresid, Kblocks);
  }
  return ERROR_OK;
})

DEFUN(if_then, {
  int err;

  CALL(exec_arg, env, end, nresid, Kcondition);
  if (env->e_value != 0)
    RETURN(exec_blocks, env, end, nresid, Kblocks);
  return ERROR_OK;
})

DEFUN(if_then_else, {
  int err;

  CALL(exec_arg, env, end, nresid, Kcondition);
  if (env->e_value != 0)
    RETURN(exec_blocks, env, end, nresid, Kthen_blocks);
  else
    RETURN(exec_blocks, env, end, nresid, Kelse_blocks);
  return ERROR_OK;
})

DEFUN(wait, {
  int err;

  CALL(exec_arg, env, end, nresid, Ksecs);
  CALL(EX_DELAY, env->e_value);
  return ERROR_OK;
})

DEFUN(wait_until, {
  int err;

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_arg, env, end, nresid, Kcondition);
    if (env->e_value != 0)
      break;
    CALL(EX_DELAY, 0.02);
  }
  return ERROR_OK;
})

DEFUN(repeat_until, {
  int err;

  for (;;) {
    CHECK_INTR(ERROR_INTERRUPTED);
    CALL(exec_arg, env, end, nresid, Kcondition);
    if (env->e_value != 0)
      break;
    CALL(exec_blocks, env, end, nresid, Kblocks);
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
  int err;
  ntype op;

  CALL(arg_keyword, end, nresid, Kop, &op);
  CALL(exec_arg, env, end, nresid, Kx);
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
  int err;
  ntype port = 0;
  ntype mode = 0;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(arg_keyword, end, nresid, Kmode, &mode);
  EX_TURN_LED(port_value(port), mode_value(mode));
  return ERROR_OK;
})

DEFUN(turn_dcmotor_on, {
  int err;
  ntype port = 0;
  ntype direction = 0;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(arg_keyword, end, nresid, Kdirection, &direction);
  EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(direction));
  return ERROR_OK;
})

DEFUN(turn_dcmotor_off, {
  int err;
  ntype port = 0;
  ntype mode = 0;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(arg_keyword, end, nresid, Kmode, &mode);
  EX_SET_DCMOTOR_MODE(port_value(port), dcmode_value(mode));
  return ERROR_OK;
})

DEFUN(buzzer_on, {
  int err;
  ntype port = 0;
  uint32_t u32;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(exec_arg, env, end, nresid, Kfrequency);
  EX_BUZZER_CONTROL(port_value(port), 1, env->e_value);
  return ERROR_OK;
})

DEFUN(buzzer_off, {
  int err;
  ntype port = 0;
  uint32_t u32;

  CALL(arg_keyword, end, nresid, Kport, &port);
  EX_BUZZER_CONTROL(port_value(port), 0, 0);
  return ERROR_OK;
})

DEFUN(set_servomotor_degree, {
  int err;
  ntype port = 0;
  uint32_t u32;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(exec_arg, env, end, nresid, Kdegree);
  EX_SERVO_MOTOR(port_value(port), env->e_value);
  return ERROR_OK;
})

DEFUN(set_dcmotor_power, {
  int err;
  ntype port = 0;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(exec_arg, env, end, nresid, Kpower);
  EX_SET_DCMOTOR_POWER(port_value(port), env->e_value);
  return ERROR_OK;
})

DEFUN(multi_led, {
  int err;

  CALL(exec_arg, env, end, nresid, Kr);
  vtype r = env->e_value;
  CALL(exec_arg, env, end, nresid, Kg);
  vtype g = env->e_value;
  CALL(exec_arg, env, end, nresid, Kb);
  vtype b = env->e_value;
  EX_MULTILED(r, g, b);
  return ERROR_OK;
})

DEFUN(servomotor_synchronized_motion, {
  int err;

  CALL(exec_arg, env, end, nresid, Kspeed);
  const int time = env->e_value;
  struct servo_sync ss[MAX_SERVOS];
  size_t count = sizeof(ss) / sizeof(ss[0]);
  CALL(init_servo_sync, env, end, nresid, ss, &count);
  EX_SERVOMOTOR_SYNCHRONIZED_MOTION(ss, count, time);
  return ERROR_OK;
})

DEFUN(3_axis_digital_accelerometer_value, {
  int err;
  ntype port = 0;
  ntype dir = 0;

  CALL(arg_keyword, end, nresid, Kport, &port);
  CALL(arg_keyword, end, nresid, Kdirection, &dir);
  env->e_value = EX_ACCELEROMETER_VALUE(port_value(port),
					acceldir_value(dir));
  return ERROR_OK;
})

DEFUN(ir_photo_reflector_value, {

  return analog_sensor_value(env, end, nresid);
})

DEFUN(light_sensor_value, {

  return analog_sensor_value(env, end, nresid);
})

DEFUN(button_value, {

  return digital_sensor_value(env, end, nresid);
})

DEFUN(touch_sensor_value, {

  return digital_sensor_value(env, end, nresid);
})

DEFUN(reset_timer, {

  EX_RESET_TIMER();
  return ERROR_OK;
})

DEFUN(timer, {

  env->e_value = EX_TIMER();
  return ERROR_OK;
})

DEFUN(pick_random, {
  int err;

  CALL(exec_arg, env, end, nresid, Kfrom);
  const vtype from = env->e_value;
  CALL(exec_arg, env, end, nresid, Kto);
  const vtype to = env->e_value;
  env->e_value = EX_RANDOM(from, to + 1);
  return ERROR_OK;
})

DEFUN(function, {

  return ERROR_OK;		/* NOP */
})

DEFUN(call_function, {
  int err;
  int32_t i32;

  CALL(arg_int, end, nresid, Kfunction, &i32);
  return exec_function(env, i32);
})

DEFUN(variable, {

  return ERROR_OK;		/* NOP */
})

DEFUN(variable_ref, {
  int err;
  uint32_t u32;

  CALL(lookup_variable, env, end, nresid, &u32);
  env->e_value = env->e_vars[u32];
  return ERROR_OK;
})

DEFUN(set_variable_to, {
  int err;
  uint32_t u32;

  CALL(lookup_variable, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kvalue);
  env->e_vars[u32] = env->e_value;
  return ERROR_OK;
})

DEFUN(change_variable_by, {
  int err;
  uint32_t u32;

  CALL(lookup_variable, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kvalue);
  env->e_vars[u32] += env->e_value;
  return ERROR_OK;
})

DEFUN(list, {

  return ERROR_OK;		/* NOP */
})

DEFUN(list_length, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  env->e_value = list_length(&env->e_lsts[u32]);
  return ERROR_OK;
})

DEFUN(list_add, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kvalue);
  list_add(&env->e_lsts[u32], env->e_value, &err);
  return list_error(err);
})

DEFUN(list_contains, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kvalue);
  env->e_value = list_contains(&env->e_lsts[u32], env->e_value);
  return ERROR_OK;
})

DEFUN(list_ref, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kposition);
  env->e_value = list_ref(&env->e_lsts[u32], env->e_value, &err);
  return list_error(err);
})

DEFUN(list_delete, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kposition);
  list_delete(&env->e_lsts[u32], env->e_value, &err);
  return list_error(err);
})

DEFUN(list_replace, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kvalue);
  const vtype value = env->e_value;
  CALL(exec_arg, env, end, nresid, Kposition);
  const vtype position = env->e_value;
  list_replace(&env->e_lsts[u32], position, value, &err);
  return list_error(err);
})

DEFUN(list_insert, {
  int err;
  uint32_t u32;

  CALL(lookup_list, env, end, nresid, &u32);
  CALL(exec_arg, env, end, nresid, Kvalue);
  const vtype value = env->e_value;
  CALL(exec_arg, env, end, nresid, Kposition);
  const vtype position = env->e_value;
  list_insert(&env->e_lsts[u32], position, value, &err);
  return list_error(err);
})
