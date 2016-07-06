/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 */

'use strict';

const biltrans = require('../biltrans');
const scripts = require('./crocodile_bite.js').scripts;
const trans = biltrans.translator(scripts);

console.log(trans.translate());
