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

'use strict';
const debug = require('debug')('mkdict');
const fs = require('fs-extra');

const insns = [
  "when-green-flag-clicked",
  "repeat",
  "forever",
  "if-then",
  "if-then-else",
  "wait",
  "wait-until",
  "repeat-until",
  "breakpoint",

  "plus",
  "minus",
  "multiply",
  "divide",
  "mod",
  "and",
  "or",
  "not",

  "equal?",
  "less-than?",
  "greater-than?",
  "less-than-or-equal?",
  "greater-than-or-equal?",

  "round",
  "math",

  "turn-led",
  "turn-dcmotor-on",
  "turn-dcmotor-off",
  "buzzer-on",
  "buzzer-off",
  "set-servomotor-degree",
  "set-dcmotor-power",
  "multi-led",
  "servomotor-synchronized-motion",

  "3-axis-digital-accelerometer-value",
  "ir-photo-reflector-value",
  "light-sensor-value",
  "button-value",
  "touch-sensor-value",

  "reset-timer",
  "timer",
  "pick-random",

  "function",
  "call-function",

  "variable",
  "variable-ref",
  "set-variable-to",
  "change-variable-by",

  "list",
  "list-length",
  "list-add",
  "list-contains?",
  "list-ref",
  "list-delete",
  "list-replace",
  "list-insert",
];

const keywords = [
  "abs",
  "sqrt",
  "sin",
  "cos",
  "tan",
  "ln",
  "log",
  "e^",
  "10^",

  "function",
  "variable",
  "list",

  "name",
  "port",
  "mode",
  "condition",
  "secs",
  "count",
  "value",
  "frequency",
  "speed",
  "degree",
  "power",
  "direction",
  "position",
  "blocks",
  "then-blocks",
  "else-blocks",
  "op",
  "from",
  "to",

  "port-settings",
  "scripts",

  "x",
  "y",
  "z",

  "r",
  "g",
  "b",

  "ON",
  "OFF",

  "NORMAL",
  "REVERSE",
  "COAST",
  "BRAKE",

  "V0",
  "V1",
  "V2",
  "V3",
  "V4",
  "V5",
  "V6",
  "V7",
  "V8",
  "V9",

  "A0",
  "A1",
  "A2",
  "A3",

  "K0",
  "K1",
  "K2",
  "K3",
  "K4",
  "K5",
  "K6",
  "K7",

  "RGB",

  "led",
  "multi-led",
  "buzzer",
  "dc-motor",
  "servo-motor",
  "push-button",
  "touch-sensor",
  "light-sensor",
  "ir-photo-reflector",
  "3-axis-digital-accelerometer",
];

const assign = (x, name) => {
  return x.reduce((acc, x, idx) => {
    if (acc[x])
      throw new Error(`${name} ${x} is already defined as ${acc[x]}`);
    acc[x] = idx + 1;
    return acc;
  }, {});
};

/*
 * Map table from insn/keyword to unique code (small integer).
 */
const insn_dict = assign(insns, "Instruction");
const keyword_dict = assign(keywords, "Keyword");

const cdict = (prefix, dict) => {
  const csym = prefix => x => x.replace(/^/, prefix)
        .replace(/-/g, '_')
        .replace(/\?/g, '')
        .replace(/\^/g, 'c');
  const sym = csym(prefix);
  return Object.keys(dict).map(x => `#define ${sym(x)} ${dict[x]}`);
};

const dump = (name, prefix, dict) => {
  const json = `../bilbinary/${name}.json`;
  fs.writeFileSync(json, JSON.stringify(dict, null, 2) + '\n');

  const cppsym = `${name.toUpperCase()}_H`
  fs.writeFileSync(`./code/${name}.h`, [].concat([
    `#if !defined(${cppsym})`,
    `#define ${cppsym}`,
  ], cdict(prefix, dict), [
    `#endif /* !defined(${cppsym}) */`,
    ''
  ]).join('\n'));
};

dump('keyword_dict', 'K', keyword_dict);
dump('insn_dict', 'I', insn_dict);
