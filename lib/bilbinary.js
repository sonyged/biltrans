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

const bson = require('bson');
const { compactify, uncompactify } = require('../lib/compactify');

const T_NUMBER = bson.BSON_DATA_NUMBER;
const T_STRING = bson.BSON_DATA_STRING;
const T_OBJECT = bson.BSON_DATA_OBJECT;
const T_ARRAY = bson.BSON_DATA_ARRAY;
const T_INT32 = bson.BSON_DATA_INT;

const byteLength = obj => Buffer.byteLength(obj, 'utf-8');

const pack_key = (b, key, type) => {
  let offset = 0;

  b.writeUInt8(type, offset);
  offset += 1;

  b.write(key, offset, 'utf-8');
  offset += byteLength(key);

  b.writeUInt8(0, offset);
  offset += 1;

  return offset;
};

const check_offset = (tag, key, obj, b, offset) => {
  if (b.length != offset) {
    const msg = `${obj} for ${$key}: ${b.length} != ${offset}`;
    throw new Error(`Failed to pack ${tag}: ${msg}`);
  }
};

const header_size = key => 1 + byteLength(key) + 1;

const pack_number = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 8);
  let offset = pack_key(b, key, T_NUMBER);

  b.writeDoubleLE(obj, offset);
  offset += 8;

  check_offset('number', key, obj, b, offset);
  return b;
};

const pack_string = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 4 + byteLength(obj) + 1);
  let offset = pack_key(b, key, T_STRING);

  b.writeUInt32LE(byteLength(obj) + 1, offset);
  offset += 4;

  b.write(obj, offset, 'utf-8');
  offset += byteLength(obj);

  b.writeUInt8(0, offset);
  offset += 1;

  check_offset('string', key, obj, b, offset);
  return b;
};

const pack_document = (obj) => {
  const bs = Object.keys(obj).map(key => pack_keyvalue(key, obj[key]));
  let b = Buffer.allocUnsafe(4 + bs.reduce((acc, x) => {
    return acc += x.length;
  }, 0) + 1);
  let offset = bs.reduce((acc, x) => {
    x.copy(b, acc);
    return acc += x.length;
  }, 4);

  b.writeUInt8(0, offset);
  offset += 1;

  b.writeUInt32LE(offset, 0);
  return b;
};

const pack_object = (type, key, obj) => {
  const doc = pack_document(obj);
  let b = Buffer.allocUnsafe(header_size(key) + doc.length);
  let offset = pack_key(b, key, type);

  doc.copy(b, offset);
  return b;
};

const pack_array = (key, obj) => {
  return pack_object(T_ARRAY, key, obj.reduce((acc, x, idx) => {
    acc[`${idx}`] = x;
    return acc;
  }, {}));
};

const int32_p = obj => {
  return Number.isInteger(obj) && obj >= -2147483648 && obj <= 2147483647;
};

const pack_int32 = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 4);
  let offset = pack_key(b, key, T_INT32);

  b.writeInt32LE(obj, offset);
  offset += 4;

  check_offset('int32', key, obj, b, offset);
  return b;
};

const pack_keyvalue = (key, obj) => {
  if (typeof obj === 'number') {
    if (int32_p(obj))
      return pack_int32(key, obj);
    return pack_number(key, obj);
  }
  if (typeof obj === 'string')
    return pack_string(key, obj);
  if (obj instanceof Array)
    return pack_array(key, obj);
  if (obj instanceof Object)
    return pack_object(T_OBJECT, key, obj);
  throw new Error(`Unsupported value: ${obj} for key ${$key}`);
};

const serialize = (obj) => {
  //process.stderr.write(JSON.stringify(obj));
  return pack_document(obj);
  const BSON = new bson();
  return BSON.serialize(obj);
};

function Translator(scripts)
{
  this.translate = () => serialize(compactify(scripts));
}

module.exports = {
  translator: function(scripts) {
    return new Translator(scripts);
  }
};
