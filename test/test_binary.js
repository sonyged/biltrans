/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

'use strict';
const bilbinary = require('../lib/bilbinary'),
      assert = require('assert');

const cp = require('child_process');
const merge_opts = (o, opts) => {
  opts = opts || {};
  o.cwd = opts.cwd;
  o.encoding = opts.encoding || 'utf-8';
  o.input = opts.input;
  return o;
};
const capture = (cmd, opts) => {
  return cp.execSync(cmd, merge_opts({}, opts)).split(/\n/);
};

describe('translate {}', function() {
  const trans = bilbinary.translator({});

  it('should be {} in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x05, 0x00, 0x00, 0x00, 0x00
    ]));
  });
});

describe('translate []', function() {
  const trans = bilbinary.translator([]);

  it('should be [] in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x05, 0x00, 0x00, 0x00, 0x00
    ]));
  });
});

describe('translate { x: {} }', function() {
  const trans = bilbinary.translator({ x: {} });

  it('should be { x: {} } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0d, 0x00, 0x00, 0x00, 0x03, 0x55,
      0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00
    ]));
  });
});

describe('translate { x: [] }', function() {
  const trans = bilbinary.translator({ x: [] });

  it('should be { x: [] } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0d, 0x00, 0x00, 0x00, 0x04, 0x55,
      0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00
    ]));
  });
});

describe('translate { x: 2 }', function() {
  const trans = bilbinary.translator({ x: 2 });

  it('should be { x: 2 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0c, 0x00, 0x00, 0x00, 0x10, 0x55,
      0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    ]));
  });
});

describe('translate { x: "x" }', function() {
  const trans = bilbinary.translator({ x: "x" });

  it('should be { x: "x" } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0a, 0x00, 0x00, 0x00, 0x02, 0x55,
      0x00, 0x55, 0x00, 0x00
    ]));
  });
});

describe('translate { x: "y" }', function() {
  const trans = bilbinary.translator({ x: "y" });

  it('should be { x: "y" } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0a, 0x00, 0x00, 0x00, 0x02, 0x55,
      0x00, 0x56, 0x00, 0x00,
    ]));
  });
});

describe('translate { x: { y: 1 } }', function() {
  const trans = bilbinary.translator({ x: { y: 1 } });

  it('should be { x: { y: 1 } } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x14, 0x00, 0x00, 0x00, 0x03, 0x55,
      0x00, 0x0c, 0x00, 0x00, 0x00, 0x10,
      0x56, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    ]));
  });
});

describe('translate { x: [ 8 ] }', function() {
  const trans = bilbinary.translator({ x: [ 8 ] });

  it('should be { x: [ 8 ] } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x14, 0x00, 0x00, 0x00, 0x04, 0x55,
      0x00, 0x0c, 0x00, 0x00, 0x00, 0x10,
      0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00
    ]));
  });
});

describe('translate { name: wait }', function() {
  const trans = bilbinary.translator({ name: 'wait' });

  it('should be { name: wait } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0a, 0x00, 0x00, 0x00, 0x02,
      0x41, 0x00, 0x06, 0x00, 0x00
    ]));
  });
});

describe('translate wait', function() {
  const trans = bilbinary.translator({ name: 'wait', secs: 0.6 });

  it('should be wait in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x11, 0x00, 0x00, 0x00, 0x02, 0x41, 0x00,
      0x06, 0x00, 0x01, 0x45, 0x00,
      0x9a, 0x99, 0x19, 0x3f, 0x00
    ]));
  });
});

describe('execute wait', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'wait', secs: 0.6 },
        { name: 'wait', secs: 1 },
        { name: 'wait', secs: { name: 'plus', x: 1, y: 0 } },
        { name: 'wait', secs: { name: 'minus', x: 0, y: 0.3 } },
        { name: 'wait', secs: { name: 'multiply', x: 3, y: 0.3 } },
        { name: 'wait', secs: { name: 'divide', x: 3, y: 0.3 } },
        { name: 'wait', secs: { name: 'divide', x: 3, y: 0 } },
        { name: 'wait', secs: { name: 'mod', x: 3, y: 0.3 } },
        { name: 'wait', secs: { name: 'mod', x: 7, y: 3 } },
        { name: 'wait', secs: { name: 'math', op: 'abs', x: -90 } },
        { name: 'wait', secs: { name: 'math', op: 'sqrt', x: 90 } },
        { name: 'wait', secs: { name: 'math', op: 'sqrt', x: -90 } },
        { name: 'wait', secs: { name: 'math', op: 'sin', x: 90 } },
        { name: 'wait', secs: { name: 'math', op: 'sin', x: -90 } },
        { name: 'wait', secs: { name: 'math', op: 'cos', x: 180 } },
        { name: 'wait', secs: { name: 'math', op: 'cos', x: -180 } },
        { name: 'wait', secs: { name: 'math', op: 'tan', x: 0 } },
        { name: 'wait', secs: { name: 'math', op: 'tan', x: 90 } },
        { name: 'wait', secs: { name: 'math', op: 'ln', x: 90 } },
        { name: 'wait', secs: { name: 'math', op: 'log', x: 90 } },
        { name: 'wait', secs: { name: 'math', op: 'e^', x: 9 } },
        { name: 'wait', secs: { name: 'math', op: '10^', x: 9 } },
      ]
    }
  ]});

  it('should execute wait in binary format', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });

    assert.deepEqual(output, [
      "wait 0.600000",
      "wait 1.000000",
      "wait 1.000000",
      "wait -0.300000",
      "wait 0.900000",
      "wait 10.000000",
      "wait inf",
      "wait nan",
      "wait 1.000000",
      "wait 90.000000",
      "wait 9.486833",
      "wait nan",
      "wait 1.000000",
      "wait -1.000000",
      "wait -1.000000",
      "wait -1.000000",
      "wait 0.000000",
      "wait -22877334.000000",
      "wait 4.499810",
      "wait 1.954243",
      "wait 8103.081543",
      "wait 1000000000.000000",
      ""
    ]);
  });
});

describe('execute repeat', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'repeat',
          count: 2,
          blocks: [
            { name: 'wait', secs: 0.6 },
            { name: 'wait', secs: 1 }
          ]
        }
      ]
    },
    { name: 'variable', variable: 'v0', value: 0 },
    { name: 'variable', variable: 'v1', value: 0 }
  ]});

  it('should execute repeat in binary format', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 0.600000",
      "wait 1.000000",
      "wait 0.600000",
      "wait 1.000000",
      ""
    ]);
  });
});

describe('execute variable', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'set-variable-to', variable: 'v0', value: 0 },
        { name: 'set-variable-to', variable: 'v1', value: 2 },
        { name: 'repeat',
          count: 3,
          blocks: [
            { name: 'wait', secs: { name: 'variable-ref', 'variable': 'v0' } },
            { name: 'wait', secs: { name: 'variable-ref', 'variable': 'v1' } },
            { name: 'change-variable-by', variable: 'v0', value: 1 },
            { name: 'change-variable-by', variable: 'v1', value: -1 },
          ]
        }
      ]
    },
    { name: 'variable', variable: 'v0', value: 0 },
    { name: 'variable', variable: 'v1', value: 0 }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 0.000000",
      "wait 2.000000",
      "wait 1.000000",
      "wait 1.000000",
      "wait 2.000000",
      "wait 0.000000",
      ""
    ]);
  });
});

describe('execute conditional', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'set-variable-to', variable: 'v0', value: 0 },
        { name: 'set-variable-to', variable: 'v1', value: 2 },
        { name: 'repeat',
          count: 3,
          blocks: [
            { name: 'if-then',
              condition: {
                name: 'equal?',
                x: { name: 'variable-ref', 'variable': 'v0' },
                y: 0
              },
              blocks: [
                { name: 'wait', secs: 1 },
              ]
            },
            { name: 'if-then-else',
              condition: {
                name: 'equal?',
                x: { name: 'variable-ref', 'variable': 'v1' },
                y: 1
              },
              'then-blocks': [
                { name: 'wait', secs: 2 },
              ],
              'else-blocks': [
                { name: 'wait', secs: 3 },
              ]
            },
            { name: 'change-variable-by', variable: 'v0', value: 1 },
            { name: 'change-variable-by', variable: 'v1', value: -1 },
          ]
        }
      ]
    },
    { name: 'variable', variable: 'v0', value: 0 },
    { name: 'variable', variable: 'v1', value: 0 }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 1.000000",
      "wait 3.000000",
      "wait 2.000000",
      "wait 3.000000",
      ""
    ]);
  });
});

describe('execute repeat-until', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'set-variable-to', variable: 'v0', value: 0 },
        { name: 'repeat-until',
          condition: {
            name: 'equal?',
            x: { name: 'variable-ref', 'variable': 'v0' },
            y: 3
          },
          blocks: [
            { name: 'wait', secs: { name: 'variable-ref', 'variable': 'v0' } },
            { name: 'change-variable-by', variable: 'v0', value: 1 },
          ]
        }
      ]
    },
    { name: 'variable', variable: 'v0', value: 0 },
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 0.000000",
      "wait 1.000000",
      "wait 2.000000",
      ""
    ]);
  });
});

describe('execute pick-random', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'set-variable-to', variable: 'v0', value: 0 },
        { name: 'set-variable-to', variable: 'v1', value: 0 },
        { name: 'wait', secs: 1 },
        { name: 'repeat-until',
          condition: {
            name: 'equal?',
            x: { name: 'variable-ref', 'variable': 'v0' },
            y: 300
          },
          blocks: [
            { name: 'set-variable-to', variable: 'v2',
              value: { name: 'pick-random', from: 2, to: 7 } },
            { name: 'if-then',
              condition: {
                name: 'less-than?',
                x: { name: 'variable-ref', 'variable': 'v2' },
                y: 2
              },
              blocks: [ { name: 'wait', secs: 2 } ]
            },
            { name: 'if-then',
              condition: {
                name: 'greater-than?',
                x: { name: 'variable-ref', 'variable': 'v2' },
                y: 7
              },
              blocks: [ { name: 'wait', secs: 3 } ]
            },
            { name: 'if-then',
              condition: {
                name: 'not',
                x: {
                  name: 'equal?',
                  x: { name: 'variable-ref', 'variable': 'v1' },
                  y: { name: 'variable-ref', 'variable': 'v2' }
                }
              },
              blocks: [
                { name: 'change-variable-by', variable: 'v0', value: 1 },
                { name: 'set-variable-to', variable: 'v1',
                  value: { name: 'variable-ref', variable: 'v2' } },
              ]
            }
          ]
        },
        { name: 'wait', secs: 9 },
      ]
    },
    { name: 'variable', variable: 'v0', value: 0 },
    { name: 'variable', variable: 'v1', value: 0 },
    { name: 'variable', variable: 'v2', value: 0 },
  ]});

  it('should return some different values', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 1.000000",
      "wait 9.000000",
      ""
    ]);
  });
});

describe('execute function', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'call-function', function: 'f0' },
        { name: 'call-function', function: 'f1' },
      ]
    },
    { name: 'function', function: 'f0',
      blocks: [ { name: 'wait', secs: 1 } ] },
    { name: 'function', function: 'f1',
      blocks: [ { name: 'wait', secs: 2 } ] },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 1.000000",
      "wait 2.000000",
      ""
    ]);
  });
});

describe('execute timer', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'reset-timer' },
        { name: 'timer' },
        { name: 'timer' },
        { name: 'reset-timer' },
        { name: 'timer' },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "reset-timer",
      "timer 0",
      "timer 1",
      "reset-timer",
      "timer 0",
      ""
    ]);
  });
});

describe('execute wait-until', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'reset-timer' },
        { name: 'wait-until',
          condition: { name: 'greater-than?', x: { name: 'timer' }, y: 5 } },
        { name: 'wait', secs: 1 },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "reset-timer",
      "timer 0",

      "wait 0.020000",
      "timer 1",
      "wait 0.020000",
      "timer 2",
      "wait 0.020000",
      "timer 3",
      "wait 0.020000",
      "timer 4",
      "wait 0.020000",
      "timer 5",
      "wait 0.020000",
      "timer 6",
      "wait 1.000000",

      ""
    ]);
  });
});

describe('execute turn-led', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'turn-led', port: 'V2', mode: 'ON' },
        { name: 'turn-led', port: 'V3', mode: 'OFF' },
        { name: 'turn-led', port: 'V4', mode: 'ON' },
        { name: 'turn-led', port: 'V5', mode: 'OFF' },
        { name: 'turn-led', port: 'V6', mode: 'ON' },
        { name: 'turn-led', port: 'V7', mode: 'OFF' },
        { name: 'turn-led', port: 'V8', mode: 'ON' },
        { name: 'turn-led', port: 'V9', mode: 'OFF' },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "turn-led 2 1",
      "turn-led 3 0",
      "turn-led 6 1",
      "turn-led 7 0",
      "turn-led 8 1",
      "turn-led 9 0",
      "turn-led 11 1",
      "turn-led 13 0",
      ""
    ]);
  });
});

describe('execute multi-led', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'multi-led', r: 0, b: 0, g: 0 },
        { name: 'multi-led',
          r: { name: 'plus', x: 10, y: 20},
          g: { name: 'minus', x: 70, y: 20},
          b: { name: 'multiply', x: 7, y: 2} },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "multi-led 0 0 0",
      "multi-led 30 50 14",
      ""
    ]);
  });
});

describe('execute dcmotor', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'turn-dcmotor-on', port: 'V0', direction: 'NORMAL' },
        { name: 'turn-dcmotor-on', port: 'V1', direction: 'REVERSE' },
        { name: 'turn-dcmotor-off', port: 'V0', mode: 'COAST' },
        { name: 'turn-dcmotor-off', port: 'V1', mode: 'BRAKE' },
        { name: 'set-dcmotor-power', port: 'V0', power: 0 },
        { name: 'set-dcmotor-power', port: 'V1',
          power: { name: 'multiply', x: 10, y: 10 } },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "dcmotor-mode 0 0",
      "dcmotor-mode 1 1",
      "dcmotor-mode 0 2",
      "dcmotor-mode 1 3",
      "dcmotor-power 0 0",
      "dcmotor-power 1 100",
      ""
    ]);
  });
});

describe('execute buzzer', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'buzzer-on', port: 'V2', frequency: 60 },
        { name: 'buzzer-on', port: 'V3',
          frequency: { name: 'plus', x: 60, y: 20} },
        { name: 'buzzer-off', port: 'V4' },
        { name: 'buzzer-off', port: 'V5' },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "buzzer-on 2 60",
      "buzzer-on 3 80",
      "buzzer-off 6",
      "buzzer-off 7",
      ""
    ]);
  });
});

describe('execute servomotor', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'set-servomotor-degree', port: 'V2', degree: 60 },
        { name: 'set-servomotor-degree', port: 'V3',
          degree: { name: 'plus', x: 60, y: 20} },
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "servomotor 2 60",
      "servomotor 3 80",
      ""
    ]);
  });
});

describe('execute servomotor-synchronized-motion', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'servomotor-synchronized-motion',
          speed: { name: 'minus', x: 8, y: 2},
          blocks: [
            { name: 'set-servomotor-degree', port: 'V2', degree: 60 },
            { name: 'set-servomotor-degree', port: 'V3',
              degree: { name: 'plus', x: 60, y: 20} },
          ]
        }
      ]
    },
  ]});

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "servomotor-synchronized-motion: time 6",
      "ss[0]: port 2 degree 60",
      "ss[1]: port 3 degree 80",
      ""
    ]);
  });
});

describe('execute digital-sensor', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A0', mode: 'ON' },
          blocks: [ { name: 'wait', secs: 1 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A1', mode: 'ON' },
          blocks: [ { name: 'wait', secs: 2 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A2', mode: 'ON' },
          blocks: [ { name: 'wait', secs: 3 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A3', mode: 'ON' },
          blocks: [ { name: 'wait', secs: 4 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A0', mode: 'OFF' },
          blocks: [ { name: 'wait', secs: 5 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A1', mode: 'OFF' },
          blocks: [ { name: 'wait', secs: 6 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A2', mode: 'OFF' },
          blocks: [ { name: 'wait', secs: 7 } ] },
        { name: 'if-then',
          condition: { name: 'button-value', port: 'A3', mode: 'OFF' },
          blocks: [ { name: 'wait', secs: 8 } ] }
      ]
    },
  ]});

  const fs = require('fs-extra');
  fs.writeFileSync('/tmp/x.bin', trans.translate());

  it('should execute functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "digital-sensor 24",
      "wait 1.000000",
      "digital-sensor 25",
      "digital-sensor 26",
      "wait 3.000000",
      "digital-sensor 27",
      "digital-sensor 24",
      "digital-sensor 25",
      "wait 6.000000",
      "digital-sensor 26",
      "digital-sensor 27",
      "wait 8.000000",
      ""
    ]);
  });
});

describe('execute port-settings', function() {
  const trans = bilbinary.translator({
    scripts: [
    ],
    "port-settings": {
      "A0": "push-button",
      "A1": "push-button",
      "RGB": "multi-led",
      "K6": "light-sensor",
      "K7": "ir-photo-reflector"
    }
  });

  const fs = require('fs-extra');
  fs.writeFileSync('/tmp/x.bin', trans.translate());

  it('should execute port-settings', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "port-init 24 7c",
      "port-init 25 7c",
      "port-init 0 29",
      "port-init 28 7e",
      "port-init 29 7f",
      ""
    ]);
  });
});
