/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

'use strict';
const { compactify, uncompactify } = require('../../bilbinary/compactify'),
      assert = require('assert');

describe('compactify empty array', function() {
  const cbil = compactify([]);

  it('should be empty array', function() {
    assert.deepEqual(cbil, []);
  });
});

describe('compactify immediate', function() {
  it('should return number if number is given', function() {
    const cbil = compactify(3.14);
    assert.deepEqual(cbil, 3.14);
  });
  it('should return string if string is given', function() {
    const cbil = compactify('3.14');
    assert.deepEqual(cbil, '3.14');
  });
});

// describe('compactify when-green-flag-clicked', function() {
//   it('should compactify empty block', function() {
//     const cbil = compactify({
//       name: 'when-green-flag-clicked',
//       blocks: []
//     });
//     assert.deepEqual(cbil, {
//       name: 1,
//       blocks: []
//     });
//   });

//   it('should compactify non empty block', function() {
//     const cbil = compactify({
//       name: 'when-green-flag-clicked',
//       blocks: [
//         { name: 'wait', secs: 0.006 }
//       ]
//     });
//     assert.deepEqual(cbil, {
//       name: 1,
//       blocks: [ { name: 6, secs: 0.006 } ]
//     });
//   });

//   it('should compactify non empty block (2)', function() {
//     const cbil = compactify({
//       name: 'when-green-flag-clicked',
//       blocks: [
//         { name: 'wait', secs: { name: 'plus', x: 3, y: -0.2 } }
//       ]
//     });
//     assert.deepEqual(cbil, {
//       name: 1,
//       blocks: [ { name: 6, secs: { name: 10, x: 3, y: -0.2 } } ]
//     });
//   });
// });

// describe('compactify when-green-flag-clicked with function', function() {
//   it('should compactify function blocks', function() {
//     const cbil = compactify({
//       scripts: [
//         {
//           name: 'when-green-flag-clicked',
//           blocks: [
//             { name: 'wait', secs: 0.006 },
//             { name: 'call-function', function: 'f' },
//             { name: 'call-function', function: 'g' }
//           ]
//         },
//         {
//           name: 'function',
//           function: 'f',
//           blocks: [
//             { name: 'call-function', function: 'g' }
//           ]
//         },
//         {
//           name: 'function',
//           function: 'g',
//           blocks: [
//             { name: 'call-function', function: 'f' }
//           ]
//         }
//       ]
//     });
//     assert.deepEqual(cbil, {
//       'port-settings': {},
//       scripts: [
//         {
//           name: 1,
//           blocks: [
//             { name: 6, secs: 0.006 },
//             { name: 52, function: 0 },
//             { name: 52, function: 1 }
//           ]
//         },
//         {
//           name: 51,
//           function: 0,
//           blocks: [
//             { name: 52, function: 1 }
//           ]
//         },
//         {
//           name: 51,
//           function: 1,
//           blocks: [
//             { name: 52, function: 0 }
//           ]
//         }
//       ]
//     });
//   });
// });
