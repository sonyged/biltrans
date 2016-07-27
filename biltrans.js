/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Program to translate from block intermediate language to C
 * language.
 */

/*
 * Typical mapping:
 *
 * TURN_LED(P, M)             board.LED((P), (M))
 * BUZZER_CONTROL(P, M, F)    board.BuzzerControl((P), (M), BHZ(F))
 * DELAY(S)                   delay((S) * 1000UL)
 * SERVO_MOTOR(P, D)          board.Servomotor((P), SVRANGE(D))
 */

'use strict';
const debug = require('debug')('biltrans');

const gensym = (() => {
  let counter = 0;
  return (prefix => { return `${prefix}${counter++}`; });
})();

function uniop(block, op)
{

  return `${op}${emit_exp(block.x)}`;
}

function binop(block, op)
{

  return [emit_exp(block.x),  op,  emit_exp(block.y)];
}

function portsym(port)
{

  return `PORT_${port}`;
}

function blkport(blk)
{

  return portsym(blk.port);
}

function symbolize(str)
{

  return str.replace(/-/g, '_');
}

function blkvar(blk)
{

  return `V${symbolize(blk.variable)}`;
}

function blkfunc(blk)
{

  return `F${symbolize(blk.function)}`;
}

function sensor(type)
{
  const conv = n => {
    return symbolize(n).toUpperCase();
  };

  return blk => {
    const port = blk.port;
    use_port(port, type);
    return `${conv(type)}(${portsym(port)})`;
  };
}

const EXPTRANS = {
  not: blk => { return uniop(blk, '!'); },
  round: blk => { return `round(${emit_exp(blk.x)})`; },

  and: blk => { return binop(blk, '&&'); },
  or: blk => { return binop(blk, '||'); },

  'greater-than?': blk => { return binop(blk, '>'); },
  'less-than?': blk => { return binop(blk, '<'); },
  'equal?': blk => { return binop(blk, '=='); },
  plus: blk => { return binop(blk, '+'); },
  minus: blk => { return binop(blk, '-'); },
  multiply: blk => { return binop(blk, '*'); },
  divide: blk => { return binop(blk, '/'); },

  mod: blk => {
    return [`(int)(${emit_exp(blk.x)})`, '%', `(int)(${emit_exp(blk.y)})`];
  },

  'variable-ref': blk => {
    return blkvar(blk);
  },

  'pick-random': blk => {
    return `random(${emit_exp(blk.from)}, (${emit_exp(blk.to)} + 1))`;
  },

  'button-value': sensor('push-button'),
  'ir-photo-reflector-value': sensor('ir-photo-reflector'),
  'light-sensor-value': sensor('light-sensor'),
  'sound-sensor-value': sensor('sound-sensor'),
  'touch-sensor-value': sensor('touch-sensor'),
};

/*
 * block-type ::= "multi-led" | "led" | "dc-motor" | "servo-motor" | "buzzer" |
 *     "light-sensor" | "touch-sensor" | "sound-sensor" |
 *      "ir-photo-reflector" | "3-axis-digital-accelerometer" | "push-button"
 */
let port_config = {};
function use_port(port, type)
{

  if (port_config[port] && port_config[port] !== type) {
    throw new Error(`Port ${port} is already used for ${port_config[port]}`);
  }
  port_config[port] = type;
}

function reduce(obj, acc, proc)
{

  return Object.keys(obj).reduce((acc, key) => {
    return proc(acc, obj, key);
  }, acc);
}

function emit_setup()
{
  const initalizer = {
    'multi-led': port => { return `INIT_MULTILED()`; },
    led: port => { return `pinMode(${port}, OUTPUT)`; },
    buzzer: port => { return `pinMode(${port}, OUTPUT)`; },
    'servo-motor': port => { return `INIT_SERVO_MOTOR(${port})`; },
    'dc-motor': port => { return `INIT_DC_MOTOR(${port})`; },
    'push-button': port => { return `INIT_PUSH_BUTTON(${port})`; },
    'ir-photo-reflector': port => { return `INIT_IR_PHOTO_REFLECTOR(${port})`; },
    '3-axis-digital-accelerometer': port => {
      return `INIT_3_AXIS_DIGITAL_ACCELEROMETER(${port})`;
    },
    'light-sensor': port => { return `INIT_LIGHT_SENSOR(${port})`; },
    'touch-sensor': port => { return `INIT_TOUCH_SENSOR(${port})`; }
  };
  const emit_config = () => {
    return reduce(port_config, [], (acc, obj, port) => {
      let proc = initalizer[obj[port]];
      return proc ? acc.concat(proc(portsym(port))) : acc;
    }).map(stmtfy);
  };
  return [].concat(
    'void setup()', '{', emit_config().map(indent), '}'
  );
}

const BLKTRANS = {
  'background-thread': blk => { return ''; },

  'when-green-flag-clicked': blk => {
    return [].concat('void loop()', '{', emit_blocks(blk.blocks), '}' );
  },

  function: blk => {
    return [].concat(
      `void ${blkfunc(blk)}()`, '{', emit_blocks(blk.blocks), '}'
    );
  },

  variable: blk => {
    return `float ${blkvar(blk)} = ${emit_exp(blk.value)}`;
  },

  forever: blk => {
    return [].concat(
      'for (;;) {',
      'CHECK_INTR;',
      emit_blocks(blk.blocks),
      '}');
  },

  'repeat-until': blk => {
    return [].concat(
      `while (!(${emit_value(blk.condition)})) {`,
      'CHECK_INTR;',
      emit_blocks(blk.blocks),
      '}');
  },

  'wait-until': blk => {
    return [].concat(
      `while (!(${emit_value(blk.condition)})) {`,
      'CHECK_INTR;',
      '  DELAY(0.02);',
      '}');
  },

  repeat: blk => {
    const v = gensym('i');
    return [].concat(
      `for (int ${v} = 0; ${v} < ${emit_value(blk.count)}; ${v}++) {`,
      'CHECK_INTR;', 
      emit_blocks(blk.blocks),
      '}');
  },

  'if-then': blk => {
    return [].concat(
      `if (${emit_value(blk.condition)}) {`, emit_blocks(blk.blocks), '}'
    );
  },

  'if-then-else': blk => {
    return [].concat(
      `if (${emit_value(blk.condition)}) {`,
      emit_blocks(blk['then-blocks']),
      '} else {',
      emit_blocks(blk['else-blocks']),
      '}'
    );
  },

  'call-function': blk => {
    return `${blkfunc(blk)}()`;
  },

  'set-variable-to': blk => {
    return `${blkvar(blk)} = ${emit_value(blk.value)}`;
  },

  'change-variable-by': blk => {
    return `${blkvar(blk)} += ${emit_value(blk.value)}`;
  },

  wait: blk => {
    return `DELAY(${emit_exp(blk.secs)})`;
  },

  'buzzer-on': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'buzzer');
    return `BUZZER_CONTROL(${port}, ON, ${blk.frequency})`;
  },

  'buzzer-off': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'buzzer');
    return `BUZZER_CONTROL(${port}, OFF, 0)`;
  },

  'turn-led': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'led');
    return `TURN_LED(${port}, ${blk.mode})`;
  },

  'multi-led': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'multi-led');
    const r = emit_value(blk.r);
    const g = emit_value(blk.g);
    const b = emit_value(blk.b);
    return `MULTILED(${r}, ${g}, ${b})`;
  },

  'set-dcmotor-power': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'dc-motor');
    return `SET_DCMOTOR_POWER(${port}, ${blk.power})`;
  },

  'turn-dcmotor-on': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'dc-motor');
    return `SET_DCMOTOR_MODE(${port}, DCMOTOR_${blk.direction})`;
  },

  'turn-dcmotor-off': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'dc-motor');
    return `SET_DCMOTOR_MODE(${port}, DCMOTOR_${blk.mode})`;
  },

  'set-servomotor-degree': blk => {
    const port = blkport(blk);
    use_port(blk.port, 'servo-motor');
    return [].concat(
      '#if !defined(SERVO_MOTOR_DEGREE)',
      '  #define SERVO_MOTOR_DEGREE(p, deg) SERVO_MOTOR((p), (deg));',
      '#endif',
      `SERVO_MOTOR_DEGREE(${port}, ${emit_value(blk.degree)})`);
  },

  'servomotor-synchronized-motion': blk => {
    return [].concat(
      '{',
      '  struct servo_sync ss[] = {',
      '  #define SERVO_MOTOR_DEGREE(p, deg) {(p), (deg)},',
      emit_blocks(blk.blocks),
      '  };',
      '  const int count = sizeof(ss) / sizeof(ss[0]);',
      `  const int time = ${emit_value(blk.speed)};`,
      '  SERVOMOTOR_SYNCHRONIZED_MOTION(ss, count, time);',
      '  #undef SERVO_MOTOR_DEGREE',
      '}');
  },

  breakpoint: blk => {
    return '/* breakpoint */';
  },
};

function indent(stmt)
{

  return `  ${stmt}`;
}

function stmtfy(exp)
{

  return `${exp};`;
}

/*
 * Translate sequence of blocks.
 */
function emit_blocks(blocks, noindent)
{
  const proc = noindent ? (x => { return x; }) : indent;

  return blocks.reduce((acc, x) => {
    return acc.concat(emit_block(x).map(proc));
  }, []);
}

/*
 * Translate single expression.
 */
function emit_exp(obj, noparen)
{

  if (typeof obj === 'string')
    return `${obj}`;
  if (typeof obj === 'number')
    return `${obj}`;
  if (typeof obj === 'boolean')
    return `${obj}`;

  const trans = EXPTRANS[obj.name];
  if (trans) {
    const exp = trans(obj);
    if (exp instanceof Array)
      return noparen ? exp.join(' ') : `(${exp.join(' ')})`;
    return exp;
  }
  return `error! ${obj.name}`;
}

function emit_value(block)
{
 
 return emit_exp(block, true);
}

/*
 * Translate single block.
 */
function emit_block(block)
{
  const trans = BLKTRANS[block.name];
  const stmt = trans ? trans(block) : emit_value(block);

  if (stmt instanceof Array)
    return stmt;                // control blocks such as for or if.
  return [ stmtfy(stmt) ];      // single statement
}

/*
 * Translate a script to list of C source lines.
 */
function emit_script(script)
{
  const blocks = (script instanceof Array) ? script : [ script ];
  return emit_blocks(blocks, true);
}

/* sort order: variable << function << others */
function compare_script(a, b)
{
  const f = x => { return x.name === 'function'; };
  const v = x => { return x.name === 'variable'; };

  if (a.name === b.name)
    return 0;
  return v(a) ? -1 : v(b) ? 1 : f(a) ? -1 : f(b) ? 1 : 0;
}

function translate_blocks(script)
{
  if (script.scripts) {
    /* Complete script */
    port_config = reduce(script['port-settings'] || {}, {}, (acc, o, key) => {
      acc[key] = o[key];
      return acc;
    });
    const decls = script.scripts.filter(x => {
      return x.name === 'function';
    }).reduce((acc, x) => {
      acc.push(`static void ${blkfunc(x)}();`);
      return acc;
    }, []);
    return script.scripts.sort(compare_script).reduce((acc, script) => {
      return acc.concat(emit_script(script));
    }, decls).concat(emit_setup()).join('\n');
  } else
    return emit_script(script).join('\n');
}

function Translator(scripts)
{
  this.translate = () => {
    return translate_blocks(scripts);
  };
}

module.exports = {
  translator: function(scripts) {
    return new Translator(scripts);
  }
};
