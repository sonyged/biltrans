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
const debug = require('debug')('compactify');
const dict = require('./keyword_dict.json');

const immediate_p = (x) => ['string', 'number', 'boolean'].includes(typeof x);

/*
 * List of blocks consists for environment.
 */
const environ_p = (x) => ['function', 'variable', 'list'].includes(x);

/*
 * List of blocks which access to environment.
 */
const environ_accessor_p = (blk) => [
  'call-function',

  'variable-ref',
  'set-variable-to',
  'change-variable-by',

  'list-length',
  'list-add',
  'list-contains?',
  'list-ref',
  'list-delete',
  'list-replace',
  'list-insert',
].includes(blk.name);

const string_key_p = (key) => [
  'name',
  'port',
  'mode',
  'direction',
].includes(key);

const compactify = (obj, fvlmap) => {
  if (obj instanceof Array)
    return obj.map(x => compactify(x, fvlmap));
  if (immediate_p(obj))
    return obj;
  return Object.keys(obj).reduce((acc, key) => {
    const v = obj[key];
    if ((environ_p(obj.name) || environ_accessor_p(obj))
        && environ_p(key)) {
      if (typeof fvlmap[key][v] !== 'number')
        throw new Error(`Unknown ${key}: ${v}`);
      acc[key] = fvlmap[key][v];
    } else {
      acc[key] = compactify(v, fvlmap);
    }
    return acc;
  }, {});
};

const compactify_toplevel = (script) => {
  const scripts = script.scripts;
  if (!scripts)
    return compactify(script, { function: {}, variable: {}, list: {} });

  let idx = { function: 0, variable: 0, list: 0 };
  return {
    'port-settings': script['port-settings'] || {},
    scripts: compactify(scripts, scripts.reduce((acc, x) => {
      if (environ_p(x.name))
        acc[x.name][x[x.name]] = idx[x.name]++;
      return acc;
    }, { function: {}, variable: {}, list: {} }))
  };
};

const uncompactify = (script) => {};
const uncompactify_toplevel = (script) => {
  const scripts = script.scripts;
  if (!scripts)
    return uncompactify(script);
  return {
    'port-settings': script['port-settings'] || {},
    script: uncompactify(scripts)
  };
};

module.exports = {
  compactify: (script) => compactify_toplevel(script),
  uncompactify: (script) => uncompactify_toplevel(script)
};
