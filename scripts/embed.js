/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 *
 */

'use strict';
const debug = require('debug')('compile');

const scripts = (() => {
  if (process.argv.length > 2) {
    const fs = require('fs');
    const scripts = fs.readFileSync(process.argv[2]);
    return JSON.parse(scripts);
  } else {
    const scripts = require('../example/empty.json').scripts;
    return scripts;
  }
})();

const bilbinary = require('../../bilbinary');
const trans = bilbinary.translator(scripts);
const buffer = trans.translate();
let i = 0;
buffer.forEach(x => {
  process.stdout.write(`0x${x.toString(16)},`);
  if ((++i) % 8 === 0)
    process.stdout.write('\n');
});
