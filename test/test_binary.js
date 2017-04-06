/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

'use strict';
const bilbinary = require('../../bilbinary'),
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
capture('cd test; make');

describe('translate {}', function() {
  const trans = bilbinary.translator({});

  it('should be {} in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x02, 0x00
    ]));
  });
});

describe('translate []', function() {
  const trans = bilbinary.translator([]);

  it('should be [] in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x02, 0x00
    ]));
  });
});

describe('translate { x: {} }', function() {
  const trans = bilbinary.translator({ x: {} });

  it('should be { x: {} } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x03, 0x56,
      0x00, 0x02, 0x00
    ]));
  });
});

describe('translate { x: [] }', function() {
  const trans = bilbinary.translator({ x: [] });

  it('should be { x: [] } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x04, 0x56,
      0x00, 0x02, 0x00
    ]));
  });
});

describe('translate { x: 2 }', function() {
  const trans = bilbinary.translator({ x: 2 });

  it('should be { x: 2 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x06, 0x00, 0x05, 0x56,
      0x00, 0x02
    ]));
  });
});

describe('translate { x: 127 }', function() {
  const trans = bilbinary.translator({ x: 127 });

  it('should be { x: 127 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x06, 0x00, 0x05, 0x56,
      0x00, 0x7f
    ]));
  });
});

describe('translate { x: -128 }', function() {
  const trans = bilbinary.translator({ x: -128 });

  it('should be { x: -128 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x06, 0x00, 0x05, 0x56,
      0x00, 0x80
    ]));
  });
});

describe('translate { x: 128 }', function() {
  const trans = bilbinary.translator({ x: 128 });

  it('should be { x: 128 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x06, 0x56,
      0x00, 0x80, 0x00
    ]));
  });
});

describe('translate { x: -129 }', function() {
  const trans = bilbinary.translator({ x: -129 });

  it('should be { x: -129 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x06, 0x56,
      0x00, 0x7f, 0xff
    ]));
  });
});

describe('translate { x: 32767 }', function() {
  const trans = bilbinary.translator({ x: 32767 });

  it('should be { x: 32767 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x06, 0x56,
      0x00, 0xff, 0x7f
    ]));
  });
});

describe('translate { x: -32768 }', function() {
  const trans = bilbinary.translator({ x: -32768 });

  it('should be { x: -32768 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x06, 0x56,
      0x00, 0x00, 0x80
    ]));
  });
});

describe('translate { x: 32768 }', function() {
  const trans = bilbinary.translator({ x: 32768 });

  it('should be { x: 32768 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      // 32bit
      // 0x09, 0x00, 0x07, 0x56,
      // 0x00, 0x00, 0x80, 0x00, 0x00
      // float
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x00, 0x00, 0x47
    ]));
  });
});

describe('translate { x: -32769 }', function() {
  const trans = bilbinary.translator({ x: -32769 });

  it('should be { x: -32769 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      // 32bit
      // 0x09, 0x00, 0x07, 0x56,
      // 0x00, 0xff, 0x7f, 0xff, 0xff
      // float
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x01, 0x00, 0xc7
    ]));
  });
});

describe('translate { x: 8388607 }', function() {
  const trans = bilbinary.translator({ x: 8388607 });

  it('should be { x: 8388607 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      // 32bit
      // 0x09, 0x00, 0x07, 0x56,
      // 0x00, 0xff, 0xff, 0x7f, 0x00
      // float
      0x09, 0x00, 0x01, 0x56,
      0x00, 0xfe, 0xff, 0xff, 0x4a
    ]));
  });
});

describe('translate { x: -8388608 }', function() {
  const trans = bilbinary.translator({ x: -8388608 });

  it('should be { x: -8388608 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      // 32bit
      // 0x09, 0x00, 0x07, 0x56,
      // 0x00, 0x00, 0x00, 0x80, 0xff
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x00, 0x00, 0xcb
    ]));
  });
});

describe('translate { x: 2147483647 }', function() {
  const trans = bilbinary.translator({ x: 2147483647 });

  it('should be { x: 2147483647 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      // 32bit
      // 0x09, 0x00, 0x07, 0x56,
      // 0x00, 0xff, 0xff, 0xff, 0x7f
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x00, 0x00, 0x4f
    ]));
  });
});

describe('translate { x: -2147483648 }', function() {
  const trans = bilbinary.translator({ x: -2147483648 });

  it('should be { x: -2147483648 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      // 32bit
      // 0x09, 0x00, 0x07, 0x56,
      // 0x00, 0x00, 0x00, 0x00, 0x80
      // float
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x00, 0x00, 0xcf
    ]));
  });
});

describe('translate { x: 2147483648 }', function() {
  const trans = bilbinary.translator({ x: 2147483648 });

  it('should be { x: 2147483648 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x00, 0x00, 0x4f
    ]));
  });
});

describe('translate { x: -2147483649 }', function() {
  const trans = bilbinary.translator({ x: -2147483649 });

  it('should be { x: -2147483649 } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x09, 0x00, 0x01, 0x56,
      0x00, 0x00, 0x00, 0x00, 0xcf
    ]));
  });
});

describe('translate { x: "x" }', function() {
  const trans = bilbinary.translator({ x: "x" });

  it('should be { x: "x" } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x02, 0x56,
      0x00, 0x56, 0x00
    ]));
  });
});

describe('translate { x: "y" }', function() {
  const trans = bilbinary.translator({ x: "y" });

  it('should be { x: "y" } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x02, 0x56,
      0x00, 0x57, 0x00
    ]));
  });
});

describe('translate { x: { y: 1 } }', function() {
  const trans = bilbinary.translator({ x: { y: 1 } });

  it('should be { x: { y: 1 } } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0b, 0x00, 0x03, 0x56,
      0x00, 0x06, 0x00, 0x05,
      0x57, 0x00, 0x01
    ]));
  });
});

describe('translate { x: [ 8 ] }', function() {
  const trans = bilbinary.translator({ x: [ 8 ] });

  it('should be { x: [ 8 ] } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x09, 0x00, 0x04, 0x56,
      0x00, 0x04, 0x00, 0x05,
      0x08
    ]));
  });
});

describe('translate { name: wait }', function() {
  const trans = bilbinary.translator({ name: 'wait' });

  it('should be { name: wait } in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x07, 0x00, 0x02,
      0x41, 0x00, 0x06, 0x00
    ]));
  });
});

describe('translate wait', function() {
  const trans = bilbinary.translator({ name: 'wait', secs: 0.6 });

  it('should be wait in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      0x0e, 0x00, 0x02, 0x41, 0x00,
      0x06, 0x00, 0x01, 0x45, 0x00,
      0x9a, 0x99, 0x19, 0x3f
    ]));
  });
});

describe('translate function', function() {
  const trans = bilbinary.translator({
    'port-settings': {},
    scripts: [
      {
        name: 'function', function: 'f', blocks: [
          { name: 'wait', secs: 0.5 },
        ]
      },
      {
        name: 'when-green-flag-clicked', blocks: [
          { name: 'call-function', function: 'f' },
          { name: 'wait', secs: {
            name: 'plus', x: 200, y: -50 }
          }
        ]
      }
    ]});

  it('should be function in binary format', function() {
    assert.deepEqual(trans.translate(), new Buffer([
      96, 0,                    // length: 96 bytes

      // 'port-settings': {}
      3,                        // type: object
      84, 0,                    // keyword: 'port-settings'
      2, 0,                     // length: 2 bytes

      // scripts: [ ...
      4,                        // type: array
      85, 0,                    // keyword: 'scripts'
      86, 0,                    // length: 86 bytes

      // { name: 'function', ... }
      3,                        // type: object
      31, 0,                    // length: 31 bytes
      2,                        // type: keyword
      65, 0,                    // keyword: 'name'
      42, 0,                    // insn: 'function'
      5,                        // type: int8
      51, 0,                    // keywrod: 'function'
      0,                        // int8: 0
      4,                        // type: array
      78, 0,                    // keyword: 'blocks'
      17, 0,                    // length: 17 bytes
      3,                        // type: object
      14, 0,                    // length: 14
      2,                        // type: keyword
      65, 0,                    // keyword: 'name'
      6, 0,                     // insn: 'wait'
      1,                        // type: float
      69, 0,                    // keyword: 'secs'
      0, 0, 0, 63,              // float: 0.5

      // { name: 'when-green-flag-clicked', ... }
      3,                        // type: object
      51, 0,                    // length: 51 bytes
      2,                        // type: keyword
      65, 0,                    // keyword: 'name'
      1, 0,                     // insn: 'when-green-flag-clicked'

      // blocks: [ ...
      4,                        // type: array
      78, 0,                    // keyword: 'blocks'
      41, 0,                    // length: 41 bytes

      // { name: 'call-function', ... }
      3,                        // type: object
      11, 0,                    // length: 11 bytes
      2,                        // type: keyword
      65, 0,                    // keyword: 'name'
      43, 0,                    // insn: 'call-function'
      5,                        // type: int8
      51, 0,                    // keyword: 'function'
      0,                        // int8: 0

      // { name: 'wait', ... }
      3,                        // type: object
      26, 0,                    // length: 26 bytes
      2,                        // type: keyword
      65, 0,                    // keyword: 'name'
      6, 0,                     // insn: 'wait'

      // secs: ...
      3,                        // type: object
      69, 0,                    // keyword: 'secs'
      16, 0,                    // length: 16 bytes
      2,                        // type: keyword
      65, 0,                    // keyword: 'name'
      10, 0,                    // insn: 'plus'
      6,                        // type: int16
      86, 0,                    // keyword: 'x'
      200, 0,                   // int16: 200
      5,                        // type: int8
      87, 0,                    // keyword: 'y'
      206,                      // int8: -50
    ]));
  });
});

describe('execute wait', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'wait', secs: 0.6 },
        { name: 'wait', secs: 1 },
        { name: 'wait', secs: -1 },
        { name: 'wait', secs: 127 }, // int8
        { name: 'wait', secs: -128 }, // int8
        { name: 'wait', secs: 128 },  // int16
        { name: 'wait', secs: -129 }, // int16
        { name: 'wait', secs: 32767 }, // int16
        { name: 'wait', secs: -32768 }, // int16
        { name: 'wait', secs: 32768 },  // float (exact integer)
        { name: 'wait', secs: -32769 }, // float (exact integer)
        { name: 'wait', secs: 8388607 },  // float (exact integer)
        { name: 'wait', secs: -8388608 }, // float (exact integer)
        { name: 'wait', secs: 2147483647 },  // float
        { name: 'wait', secs: -2147483648 }, // float
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
        { name: 'if-then', condition: {
          name: 'greater-than-or-equal?',
          x: 5,
          y: { name: 'plus', x: 2, y: 3 }
        }, blocks: [ { name: 'wait', secs: 0.123 }, ]
        },
        { name: 'if-then', condition: {
          name: 'greater-than-or-equal?',
          x: 5.5,
          y: { name: 'plus', x: 2, y: 3 }
        }, blocks: [ { name: 'wait', secs: 0.456 }, ]
        },
        { name: 'if-then', condition: {
          name: 'not',
          x: {
            name: 'greater-than-or-equal?',
            x: 4.5,
            y: { name: 'plus', x: 2, y: 3 }
          }
        }, blocks: [ { name: 'wait', secs: 0.789 }, ]
        },
        { name: 'if-then', condition: {
          name: 'less-than-or-equal?',
          x: 5,
          y: { name: 'plus', x: 2, y: 3 }
        }, blocks: [ { name: 'wait', secs: 0.321 }, ]
        },
        { name: 'if-then', condition: {
          name: 'less-than-or-equal?',
          x: 4.5,
          y: { name: 'plus', x: 2, y: 3 }
        }, blocks: [ { name: 'wait', secs: 0.654 }, ]
        },
        { name: 'if-then', condition: {
          name: 'not',
          x: {
            name: 'less-than-or-equal?',
            x: 5.5,
            y: { name: 'plus', x: 2, y: 3 }
          }
        }, blocks: [ { name: 'wait', secs: 0.987 }, ]
        }
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
      "wait -1.000000",
      "wait 127.000000",
      "wait -128.000000",
      "wait 128.000000",
      "wait -129.000000",
      "wait 32767.000000",
      "wait -32768.000000",
      "wait 32768.000000",
      "wait -32769.000000",
      "wait 8388607.000000",
      "wait -8388608.000000",
      "wait 2147483648.000000", // not 2147483647 since already inexact.
      "wait -2147483648.000000",
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
      "wait 0.123000",
      "wait 0.456000",
      "wait 0.789000",
      "wait 0.321000",
      "wait 0.654000",
      "wait 0.987000",
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

describe('execute list', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'wait', secs: { name: 'list-length', 'list': 'l0' } },
        { name: 'list-add', 'list': 'l0', value: 1 },
        { name: 'wait', secs: { name: 'list-length', 'list': 'l0' } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 1
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: {
            name: 'plus', x: 1, y: 0
          }
        } },
        { name: 'list-add', 'list': 'l1', value: -1 },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 1
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: {
            name: 'plus', x: 0, y: 1
          }
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 99
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l1', value: -1
        } },
        { name: 'list-delete', 'list': 'l0', position: 1 },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 1
        } },
        { name: 'wait', secs: { name: 'list-length', 'list': 'l0' } },
        { name: 'list-add', 'list': 'l0', value: 5 },
        { name: 'list-add', 'list': 'l0', value: {
          name: 'plus', x: 2, y: 4
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 5
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 6
        } },
        { name: 'list-replace', 'list': 'l0', position: 1, value: {
          name: 'plus', x: 3, y: 4
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 5
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 6
        } },
        { name: 'wait', secs: {
          name: 'list-contains?', 'list': 'l0', value: 7
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 1
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 2
        } },
        { name: 'list-insert', 'list': 'l0', position: 1, value: {
          name: 'plus', x: 4, y: 4
        } },
        { name: 'list-insert', 'list': 'l0', position: 3, value: {
          name: 'plus', x: 4, y: 2.5
        } },
        { name: 'list-insert', 'list': 'l0', position: 5, value: {
          name: 'plus', x: 3, y: 2.5
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 1
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 2
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 3
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 4
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l0', position: 5
        } },
        { name: 'list-delete', 'list': 'l1', position: 1 },
        { name: 'list-insert', 'list': 'l1', position: 1, value: 1 },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 1
        } },
        { name: 'list-insert', 'list': 'l1', position: 2, value: 2 },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 1
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 2
        } },
        { name: 'list-insert', 'list': 'l1', position: 1, value: 3 },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 1
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 2
        } },
        { name: 'wait', secs: {
          name: 'list-ref', 'list': 'l1', position: 3
        } },
      ]
    },
    { name: 'list', list: 'l0', value: [] },
    { name: 'list', list: 'l1', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "wait 0.000000",
      "wait 1.000000",
      "wait 1.000000",
      "wait 1.000000",
      "wait -1.000000",
      "wait 1.000000",
      "wait 0.000000",
      "wait 1.000000",
      "wait 0.000000",
      "wait 0.000000",
      "wait 1.000000",
      "wait 1.000000",
      "wait 0.000000",
      "wait 1.000000",
      "wait 1.000000",
      "wait 7.000000",
      "wait 6.000000",
      "wait 8.000000",
      "wait 7.000000",
      "wait 6.500000",
      "wait 6.000000",
      "wait 5.500000",
      "wait 1.000000",
      "wait 1.000000",
      "wait 2.000000",
      "wait 3.000000",
      "wait 1.000000",
      "wait 2.000000",
      ""
    ]);
  });
});

describe('execute list (list-ref error)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [ { name: 'list-ref', 'list': 'l0', position: 1 } ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-ref error 2)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-ref', 'list': 'l0', position: 0 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-ref error 3)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-ref', 'list': 'l0', position: 2 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-delete error)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-delete', 'list': 'l0', position: 1 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-delete error 2)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-delete', 'list': 'l0', position: 0 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-delete error 3)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-delete', 'list': 'l0', position: 2 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-replace error)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-replace', 'list': 'l0', position: 1, value: 1 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-replace error 2)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-replace', 'list': 'l0', position: 0, value: 0 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-replace error 3)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-replace', 'list': 'l0', position: 2, value: 2 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-insert error)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-insert', 'list': 'l0', position: 0, value: 1 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-insert error 2)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-insert', 'list': 'l0', position: 2, value: 1 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-insert error 3)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-insert', 'list': 'l0', position: 0, value: 0 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
      ""
    ]);
  });
});

describe('execute list (list-insert error 4)', function() {
  const trans = bilbinary.translator({ scripts: [
    { name: 'when-green-flag-clicked',
      blocks: [
        { name: 'list-add', 'list': 'l0', value: 3 },
        { name: 'list-insert', 'list': 'l0', position: 3, value: 2 }
      ]
    },
    { name: 'list', list: 'l0', value: [] }
  ]});

  it('should execute variable related functions', function() {
    const output = capture('./test/test_binary', {
      input: trans.translate()
    });
    assert.deepEqual(output, [
      "got error 10",
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
      "port-init 24 7d",
      "port-init 25 7d",
      "port-init 0 29",
      "port-init 28 7f",
      "port-init 29 80",
      ""
    ]);
  });
});
