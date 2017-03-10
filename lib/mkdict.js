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

const keywords = [
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
  "abs",
  "sqrt",
  "sin",
  "cos",
  "tan",
  "ln",
  "log",
  "e^",
  "10^",

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

const dict = keywords.reduce((acc, x, idx) => {
  acc[x] = idx + 1;
  return acc;
}, {});

const csym = x => x.replace(/^/, 'S')
      .replace(/-/g, '_')
      .replace(/\?/g, '')
      .replace(/\^/g, 'c');
const cdict = keywords.map(x => `#define ${csym(x)} ${dict[x]}`);

fs.writeFileSync('./lib/string_dict.json', JSON.stringify(dict));
fs.writeFileSync('./code/string_dict.h', [].concat([
  '#if !defined(STRING_DICT_H)',
  '#define STRING_DICT_H',
], cdict, [
  '#endif /* !defined(STRING_DICT_H) */',
  ''
]).join('\n'));
