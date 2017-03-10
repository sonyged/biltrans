/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
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
      expect: 'for (;;) {\nCHECK_INTR();\n}' },
    { block: { name: 'forever', blocks: [
      { name: 'turn-led', port: 'A4', mode: 'ON' }
    ] },
      expect: 'for (;;) {\nCHECK_INTR();\n  TURN_LED(PORT_A4, ON);\n}' },
    { block: { name: 'forever', blocks: [
      { name: 'turn-led', port: 'A4', mode: 'ON' },
      { name: 'turn-led', port: 'A5', mode: 'OFF' }
    ] },
      expect: `\
for (;;) {
CHECK_INTR();
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
CHECK_INTR();
  for (int i1 = 0; i1 < 3; i1++) {
  CHECK_INTR();
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
      expect: 'Fsym2();'
    },
    { block: { name: 'variable-ref', variable: 'bite' },
      expect: 'Vsym3;'
    },
    { block: { name: 'set-variable-to', variable: 'bite',
	       value: { name: 'not',
			x: { name: 'variable-ref', variable: 'bite' } } },
      expect: 'Vsym3 = !Vsym3;'
    },
    { block: { name: 'change-variable-by', variable: 'bite',
	       value: { name: 'plus',
			x: { name: 'variable-ref', variable: 'bite' },
			y: 20 } },
      expect: 'Vsym3 += Vsym3 + 20;'
    },
    {
      block: { name: 'variable', variable: 'bite', value: 0 },
      expect: 'float Vsym3 = 0;'
    },
    {
      block: { name: 'function', function: 'ok', blocks: [] },
      expect: 'void Fsym2()\n{\n}'
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
static void Fsym2();
static void Fsym4();
float Vsym5 = 0;
float Vsym6 = 0;
void Fsym2()
{
}
void Fsym4()
{
}
void loop()
{
  setup();
  for (;;) {
  CHECK_INTR();
  }
}
void setup()
{
  RESET_TIMER();
}`
    },
    {
      block: { name: 'list', list: 'lst' },
      expect: 'void *Lsym7 = 0;'
    },
    {
      block: { name: 'plus', x: { name: 'list-length', list: 'lst' }, y: 5 },
      expect: 'LIST_LENGTH(Lsym7) + 5;'
    },
    {
      block: { name: 'list-contains?', list: 'lst',
               value: { name: 'plus', x: 8, y: 7 } },
      expect: 'LIST_CONTAINS(Lsym7, (8 + 7));'
    },
    {
      block: { name: 'list-ref', list: 'lst',
               position: { name: 'plus', x: 8, y: 7 } },
      expect: 'LIST_REF(Lsym7, (8 + 7));'
    },
    {
      block: { name: 'list-add', list: 'lst',
               value: { name: 'plus', x: 8, y: 7 } },
      expect: 'LIST_ADD(Lsym7, (8 + 7));'
    },
    {
      block: { name: 'list-delete', list: 'lst',
               position: { name: 'plus', x: 8, y: 7 } },
      expect: 'LIST_DELETE(Lsym7, (8 + 7));'
    },
    {
      block: { name: 'list-replace', list: 'lst',
               position: { name: 'plus', x: 8, y: 7 },
               value: { name: 'minus', x: 6, y: 3 }},
      expect: 'LIST_REPLACE(Lsym7, (8 + 7), (6 - 3));'
    },
    {
      block: { name: 'list-insert', list: 'lst',
               position: { name: 'multiply', x: 8, y: 7 },
               value: { name: 'divide', x: 6, y: 3 }},
      expect: 'LIST_INSERT(Lsym7, (8 * 7), (6 / 3));'
    },
    {
      block: { name: 'when-green-flag-clicked',
               blocks: [
                 { name: 'variable', variable: 'same', value: 0 },
                 { name: 'function', function: 'same', blocks: [] },
                 { name: 'list', list: 'same' },
               ] },
      expect: `\
void loop()
{
  float Vsym8 = 0;
  void Fsym9()
  {
  }
  void *Lsym10 = 0;
  setup();
  for (;;) {
  CHECK_INTR();
  }
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
