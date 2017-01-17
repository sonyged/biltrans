/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
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

let symbols = {};

function intern(prefix, name)
{
  const sym = symbols[name];

  if (sym)
    return sym;

  symbols[name] = gensym(`${prefix}sym`);
  return symbols[name];
}

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

  return intern('V', blk.variable);
}

function blkfunc(blk)
{

  return intern('F', blk.function);
}

function blklist(blk)
{

  return intern('L', blk.list);
}

function sensor_name(n)
{

  return symbolize(n).toUpperCase();
}

function analog_sensor(type)
{

  return blk => {
    const port = blk.port;
    use_port(port, type);
    return `${sensor_name(type)}(${portsym(port)})`;
  };
}

function digital_sensor(type)
{

  return blk => {
    const port = blk.port;
    const value = `${sensor_name(type)}(${portsym(port)})`;
    use_port(port, type);

    if (blk.mode === 'ON')
      return `(${value} == 0)`;
    if (blk.mode === 'OFF')
      return `(${value} != 0)`;
    return value;
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

  math: blk => {
    const deg2rad = x => `((${x}) / 180.0 * M_PI)`;
    if (blk.op === 'abs') {
      return `(float)fabs(${emit_exp(blk.x)})`;
    }
    if (blk.op === 'sqrt') {
      return `(float)sqrt(${emit_exp(blk.x)})`;
    }
    if (blk.op === 'sin') {
      return `(float)sin(${deg2rad(emit_exp(blk.x))})`;
    }
    if (blk.op === 'cos') {
      return `(float)cos(${deg2rad(emit_exp(blk.x))})`;
    }
    if (blk.op === 'tan') {
      return `(float)tan(${deg2rad(emit_exp(blk.x))})`;
    }
    if (blk.op === 'ln') {
      return `(float)log(${emit_exp(blk.x)})`
    }
    if (blk.op === 'log') {
      return `(float)log10(${emit_exp(blk.x)})`
    }
    if (blk.op === 'e^') {
      return `(float)exp(${emit_exp(blk.x)})`
    }
    if (blk.op === '10^') {
      return `(float)pow(10, ${emit_exp(blk.x)})`
    }
    return `error! ${blk.name}(${blk.op})`;
  },

  'variable-ref': blk => {
    return blkvar(blk);
  },

  'list-length': blk => {
    return `LIST_LENGTH(${blklist(blk)})`;
  },

  'list-contains?': blk => {
    return `LIST_CONTAINS(${blklist(blk)}, ${emit_exp(blk.value)})`;
  },

  'list-ref': blk => {
    return `LIST_REF(${blklist(blk)}, ${emit_exp(blk.position)})`;
  },

  'pick-random': blk => {
    return `random(${emit_exp(blk.from)}, (${emit_exp(blk.to)} + 1))`;
  },

  'button-value': digital_sensor('push-button'),
  'touch-sensor-value': digital_sensor('touch-sensor'),

  'ir-photo-reflector-value': analog_sensor('ir-photo-reflector'),
  'light-sensor-value': analog_sensor('light-sensor'),
  'sound-sensor-value': analog_sensor('sound-sensor'),

  '3-axis-digital-accelerometer-value': blk => {
    const port = blk.port;
    const direction =
          blk.direction === 'x' ? 1 :
          blk.direction === 'y' ? 2 : 3;
    use_port(port, '3-axis-digital-accelerometer');
    return `ACCELEROMETER_VALUE(${portsym(port)}, ${direction})`;
  },

  timer: blk => {
    return 'TIMER()';
  }
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
    buzzer: port => { return `BUZZER_CONTROL(${port}, OFF, 0)`; },
    'servo-motor': port => { return `INIT_SERVO_MOTOR(${port})`; },
    'dc-motor': port => { return `INIT_DC_MOTOR(${port})`; },
    'push-button': port => { return `INIT_PUSH_BUTTON(${port})`; },
    'ir-photo-reflector': port => {
      return `INIT_IR_PHOTO_REFLECTOR(${port})`;
    },
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
    'void setup()', '{',
    emit_config().map(indent),
    [ 'RESET_TIMER();' ].map(indent),
    '}'
  );
}

function emit_forever(blks)
{
  return [].concat(
    'for (;;) {',
    'CHECK_INTR;',
    blks,
    '}');
}

const BLKTRANS = {
  'background-thread': blk => { return ''; },

  'when-green-flag-clicked': blk => {
    return [].concat(
      'void loop()', '{',
      emit_blocks(blk.blocks),
      /* if above blocks exits, loop forever */
      [ 'setup();' ].concat(emit_forever([])).map(indent),
      '}'
    );
  },

  /*
   * Ignore fragment pseudo block.  It is a placeholder to hold work
   * in progress blocks.
   */
  fragment: blk => [],

  function: blk => {
    return [].concat(
      `void ${blkfunc(blk)}()`, '{', emit_blocks(blk.blocks), '}'
    );
  },

  variable: blk => {
    return `float ${blkvar(blk)} = ${emit_exp(blk.value)}`;
  },

  list: blk => {
    return `void *${blklist(blk)} = 0`;
  },

  'list-add': blk => {
    const value = emit_exp(blk.value);
    return `LIST_ADD(${blklist(blk)}, ${value})`;
  },

  'list-delete': blk => {
    const position = emit_exp(blk.position);
    return `LIST_DELETE(${blklist(blk)}, ${position})`;
  },

  'list-replace': blk => {
    const position = emit_exp(blk.position);
    const value = emit_exp(blk.value);
    return `LIST_REPLACE(${blklist(blk)}, ${position}, ${value})`;
  },

  'list-insert': blk => {
    const position = emit_exp(blk.position);
    const value = emit_exp(blk.value);
    return `LIST_INSERT(${blklist(blk)}, ${position}, ${value})`;
  },

  forever: blk => {
    return emit_forever(emit_blocks(blk.blocks));
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
    return `BUZZER_CONTROL(${port}, ON, ${emit_value(blk.frequency)})`;
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
    return `SET_DCMOTOR_POWER(${port}, ${emit_value(blk.power)})`;
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

  'reset-timer': blk => {
    return 'RESET_TIMER()';
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

/* sort order: variable|list << function << others */
function compare_script(a, b)
{
  const f = x => { return x.name === 'function'; };
  const v = x => { return x.name === 'variable' || x.name === 'list'; };

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
