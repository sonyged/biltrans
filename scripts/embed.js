/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 *
 */

'use strict';
const debug = require('debug')('compile');
const fs = require('fs');
const path = require('path');

const scripts_dir = path.dirname(process.argv[1]);
const scripts = (() => {
  if (process.argv.length > 2) {
    const scripts = fs.readFileSync(process.argv[2]);
    return JSON.parse(scripts);
  } else {
    const scripts = require('../example/empty.json').scripts;
    return scripts;
  }
})();

const version_path = `${scripts_dir}/../code/koov_version.h`;
const koov_version = fs.readFileSync(version_path, 'ascii');
const re = /#define KOOV_VERSION\s+"koov-([^"]*)"/;
const current_version = koov_version.match(re)[1];
if (current_version !== scripts.version) {
  process.stderr.write(`version ${scripts.version} != ${current_version}\n`);
  process.exit(15);
}

const bilbinary = require('../../bilbinary');
const trans = bilbinary.translator(scripts);
const buffer = trans.translate();
let i = 0;
buffer.forEach(x => {
  const hex = x.toString(16);
  const pad = hex.length === 1 ? '0' : '';
  process.stdout.write(`0x${pad}${hex},`);
  if ((++i) % 8 === 0)
    process.stdout.write('\n');
  else
    process.stdout.write(' ');
});
