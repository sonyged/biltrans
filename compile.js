/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 *
 */

'use strict';
const debug = require('debug')('compile');
const biltrans = require('./biltrans.js');

const scripts = (() => {
  if (process.argv.length > 2) {
    const fs = require('fs');
    const scripts = fs.readFileSync(process.argv[2]);
    return JSON.parse(scripts);
  } else {
    const scripts = require('./example/example2.js').scripts;
    return scripts;
  }
})();

const trans = biltrans.translator(scripts);
console.log(trans.translate());

