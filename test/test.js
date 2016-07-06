/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 */

'use strict';
var biltrans = require('../biltrans'),
    assert = require('assert');

describe('translate number', function() {
  var trans = biltrans.translator(3);

  it('should be "3"', function() {
    assert.equal(trans.translate(), "3;");
  });
});

describe('translate operator', function() {
  const tests = [
    { block: { name: 'not', x: false }, expect: '!false' },
    { block: { name: 'round', x: 3.5 }, expect: 'round(3.5)' },

    { block: { name: 'and', x: true, y: false }, expect: 'true && false' },
    { block: { name: 'or', x: false, y: true }, expect: 'false || true' },

    { block: { name: 'greater-than?', x: 3, y: 5 }, expect: '3 > 5' },
    { block: { name: 'less-than?', x: 3, y: 5 }, expect: '3 < 5' },
    { block: { name: 'equal?', x: 3, y: 5 }, expect: '3 == 5' },
    { block: { name: 'plus', x: 3, y: 5 }, expect: '3 + 5' },
    { block: { name: 'minus', x: 3, y: 5 }, expect: '3 - 5' },
    { block: { name: 'multiply', x: 3, y: 5 }, expect: '3 * 5' },
    { block: { name: 'divide', x: 3, y: 5 }, expect: '3 / 5' },
    { block: { name: 'divide',
	       x: { name: 'plus', x: 3, y: 5 },
	       y: { name: 'minus', x: 3, y: 5 } },
      expect: '(3 + 5) / (3 - 5)' },

    { block: { name: 'mod', x: 3.5, y: 5.7 },
      expect: '(int)(3.5) % (int)(5.7)' },
    { block: { name: 'equal?',
	       x: { name: 'mod', x: 3.5, y: 5.7 },
	       y: 3 },
      expect: '((int)(3.5) % (int)(5.7)) == 3' },
  ];

  tests.forEach(test => {
    let trans = biltrans.translator(test.block);
    it(`should be "${test.expect}"`, function() {
      assert.equal(trans.translate(), test.expect + ';');
    });
  });
});

describe('translate controls', function() {
  const tests = [
    { block: { name: 'forever', blocks: [] },
      expect: 'for (;;) {\nCHECK_INTR;\n}' },
    { block: { name: 'forever', blocks: [
      { name: 'turn-led', port: 'A4', mode: 'ON' }
    ] },
      expect: 'for (;;) {\nCHECK_INTR;\n  TURN_LED(PORT_A4, ON);\n}' },
    { block: { name: 'forever', blocks: [
      { name: 'turn-led', port: 'A4', mode: 'ON' },
      { name: 'turn-led', port: 'A5', mode: 'OFF' }
    ] },
      expect: `\
for (;;) {
CHECK_INTR;
  TURN_LED(PORT_A4, ON);
  TURN_LED(PORT_A5, OFF);
}` },
    { block: { name: 'repeat', count: 5, blocks: [
      { name: 'repeat', count: 3, blocks: [
	{ name: 'turn-led', port: 'A4', mode: 'ON' },
	{ name: 'turn-led', port: 'A5', mode: 'OFF' }
      ] }
    ] },
      expect: `\
for (int i0 = 0; i0 < 5; i0++) {
CHECK_INTR;
  for (int i1 = 0; i1 < 3; i1++) {
  CHECK_INTR;
    TURN_LED(PORT_A4, ON);
    TURN_LED(PORT_A5, OFF);
  }
}` },
    { block: { name: 'if-then',
	       condition: { name: 'equal?',
			    x: { name: 'plus',
				 x: 3,
				 y: { name: 'minus', x: 8, y: 3 } },
			    y: 8 },
	       blocks: [] },
      expect: 'if ((3 + (8 - 3)) == 8) {\n}'
    },
    { block: { name: 'if-then-else', condition: false,
	       'then-blocks': [], 'else-blocks': [] },
      expect: 'if (false) {\n} else {\n}'
    },
    { block: { name: 'call-function', function: 'ok' },
      expect: 'Fok();'
    },
    { block: { name: 'variable-ref', variable: 'bite' },
      expect: 'Vbite;'
    },
    { block: { name: 'set-variable-to', variable: 'bite',
	       value: { name: 'not',
			x: { name: 'variable-ref', variable: 'bite' } } },
      expect: 'Vbite = !Vbite;'
    },
    { block: { name: 'change-variable-by', variable: 'bite',
	       value: { name: 'plus',
			x: { name: 'variable-ref', variable: 'bite' },
			y: 20 } },
      expect: 'Vbite += Vbite + 20;'
    },
    {
      block: { name: 'variable', variable: 'bite', value: 0 },
      expect: 'float Vbite = 0;'
    },
    {
      block: { name: 'function', function: 'ok', blocks: [] },
      expect: 'void Fok()\n{\n}'
    },
    {
      block: { name: 'wait', secs: 0.2 },
      expect: 'DELAY(0.2);'
    },
    {
      block: {
	scripts: [
	  { name: 'function', function: 'ok', blocks: [] },
	  { name: 'variable', variable: 'x', value: 0 },
	  { name: 'when-green-flag-clicked', blocks: [] },
	  { name: 'function', function: 'ng', blocks: [] },
	  { name: 'variable', variable: 'y', value: 0 },
	]
      },
      expect: `\
static void Fok();
static void Fng();
float Vx = 0;
float Vy = 0;
void Fok()
{
}
void Fng()
{
}
void loop()
{
}
void setup()
{
}`
    },
  ];

  tests.forEach(test => {
    let trans = biltrans.translator(test.block);
    it(`should be "${test.expect}"`, function() {
      assert.equal(trans.translate(), test.expect);
    });
  });
});
