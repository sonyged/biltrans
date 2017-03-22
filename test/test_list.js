/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

'use strict';
const assert = require('assert');

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

describe('execute listtest', function() {
  it('should execute listtest successfully', function() {
    const output = capture('./test/test_list');

    assert.deepEqual(output, [
      ": []",
      "LIST_ADD(l, 3): [3.000000, ]",
      "LIST_ADD(l, 2): [3.000000, 2.000000, ]",
      "LIST_ADD(l, 2): [3.000000, 2.000000, 2.000000, ]",
      "LIST_REPLACE(l, 1, 4): [3.000000, 4.000000, 2.000000, ]",
      "LIST_DELETE(l, 0): [4.000000, 2.000000, ]",
      "LIST_INSERT(l, 0, 1): [1.000000, 4.000000, 2.000000, ]",
      "LIST_INSERT(l, 1, 3): [1.000000, 3.000000, 4.000000, 2.000000, ]",
      "LIST_DELETE(l, 4): [1.000000, 3.000000, 4.000000, 2.000000, ]",
      "LIST_DELETE(l, 3): [1.000000, 3.000000, 4.000000, ]",
      "LIST_DELETE(l, 1): [1.000000, 4.000000, ]",
      "LIST_INSERT(l, 2, 1): [1.000000, 4.000000, 1.000000, ]",
      "LIST_DELETE(l, 2): [1.000000, 4.000000, ]",
      "LIST_DELETE(l, 0): [4.000000, ]",
      "LIST_DELETE(l, 0): []",
      "LIST_DELETE(l, 0): []",
      "",
    ]);
  });
});
