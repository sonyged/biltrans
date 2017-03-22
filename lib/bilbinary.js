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

const { compactify, uncompactify } = require('../lib/compactify');
const dict = require('./keyword_dict.json');

const BT_NUMBER = 0x01;         // floating point number
const BT_KEYWORD = 0x02;        // keyword
const BT_OBJECT = 0x03;         // json object
const BT_ARRAY = 0x04;          // json array
const BT_INT8 = 0x05;           // 8 bit integer
const BT_INT16 = 0x06;          // 16 bit integer
const BT_INT32 = 0x07;          // 32 bit integer

const byteLength = obj => Buffer.byteLength(obj, 'utf-8');

const { keyLength, keyEncode } = (() => {
  return {
    keyLength: key => key ? 2 : 0,
    keyEncode: (b, key, offset) => {
      if (key) {
        let code = dict[key];
        if (!code) {
          code = parseInt(key);
          if (`${code}` !== key)
            throw new Error(`Failed to convert key ${key} to code`);
        }
        if (code > 65535)
          throw new Error(`Code ${code} for keyword ${key} too big`);
        b.writeUInt16LE(code, offset);
      }
    }
  };
})();

const pack_key = (b, key, type) => {
  let offset = 0;

  b.writeUInt8(type, offset);
  offset += 1;

  keyEncode(b, key, offset);
  offset += keyLength(key);

  return offset;
};

const header_size = key => 1 + keyLength(key); // type followed by key

const elist_nul = false;
const elist_size = (s) => {
  if (elist_nul)
    return s + 1;
  return s;
};

const check_offset = (tag, key, obj, b, offset) => {
  if (b.length != offset) {
    const msg = `${obj} for ${key}: ${b.length} != ${offset}`;
    throw new Error(`Failed to pack ${tag}: ${msg}`);
  }
};

const pack_number = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 4);
  let offset = pack_key(b, key, BT_NUMBER);

  b.writeFloatLE(obj, offset);
  offset += 4;

  check_offset('number', key, obj, b, offset);
  return b;
};

const pack_keyword = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 2);
  let offset = pack_key(b, key, BT_KEYWORD);
  const code = dict[obj];

  if (!code)
    throw new Error(`Failed to convert value ${obj} to code`);
  b.writeUInt16LE(code, offset);
  offset += 2;

  check_offset('keyword', key, obj, b, offset);
  return b;
};

const pack_elist = (bs) => {
  //const hsize = 4;
  const hsize = 2;
  const bslen = bs.reduce((acc, x) => acc + x.length, 0);
  let b = Buffer.allocUnsafe(elist_size(hsize + bslen));
  let offset = bs.reduce((acc, x) => {
    x.copy(b, acc);
    return acc + x.length;
  }, hsize);

  if (elist_nul) {
    b.writeUInt8(0, offset);
    offset += 1;
  }

  if (hsize === 4)
    b.writeUInt32LE(offset, 0);
  else
    b.writeUInt16LE(offset, 0);
  return b;
}

const pack_document = (obj, array) => {
  const bs = Object.keys(obj).map(key => pack_keyvalue(key, obj[key]));
  return pack_elist(bs);
};

const pack_object = (type, key, obj, array) => {
  const doc = pack_document(obj, array);
  let b = Buffer.allocUnsafe(header_size(key) + doc.length);
  const offset = pack_key(b, key, type);

  doc.copy(b, offset);
  return b;
};

const pack_array = (key, obj) => {
  // return pack_object(BT_ARRAY, key, obj.reduce((acc, x, idx) => {
  //   acc[`${idx}`] = x;
  //   return acc;
  // }, {}), true);

  const bs = obj.map(x => pack_keyvalue(null, x));
  const doc = pack_elist(bs);
  const type = BT_ARRAY;
  let b = Buffer.allocUnsafe(header_size(key) + doc.length);
  const offset = pack_key(b, key, type);

  doc.copy(b, offset);
  return b;
};

const { int8_p, int16_p, int32_p } = (() => {
  return [
    { name: 'int8_p', min: -128, max: 127 },
    { name: 'int16_p', min: -32768, max: 32767 },
    { name: 'int32_p', min: -2147483648, max: 2147483647 }
  ].reduce((acc, x) => {
    acc[x.name] = obj => {
      return Number.isInteger(obj) && obj >= x.min && obj <= x.max;
    };
    return acc;
  }, {});
})();

const pack_int8 = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 1);
  let offset = pack_key(b, key, BT_INT8);

  b.writeInt8(obj, offset);
  offset += 1;

  check_offset('int8', key, obj, b, offset);
  return b;
};

const pack_int16 = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 2);
  let offset = pack_key(b, key, BT_INT16);

  b.writeInt16LE(obj, offset);
  offset += 2;

  check_offset('int16', key, obj, b, offset);
  return b;
};

const pack_int32 = (key, obj) => {
  let b = Buffer.allocUnsafe(header_size(key) + 4);
  let offset = pack_key(b, key, BT_INT32);

  b.writeInt32LE(obj, offset);
  offset += 4;

  check_offset('int32', key, obj, b, offset);
  return b;
};

const pack_keyvalue = (key, obj) => {
  if (typeof obj === 'number') {
    if (int8_p(obj))
      return pack_int8(key, obj);
    if (int16_p(obj))
      return pack_int16(key, obj);
    if (int32_p(obj))
      return pack_int32(key, obj);
    return pack_number(key, obj);
  }
  if (typeof obj === 'string')
    return pack_keyword(key, obj);
  if (obj instanceof Array)
    return pack_array(key, obj);
  if (obj instanceof Object)
    return pack_object(BT_OBJECT, key, obj, false);
  throw new Error(`Unsupported value: ${obj} for key ${$key}`);
};

const serialize = (obj) => {
  //process.stderr.write(JSON.stringify(obj));
  return pack_document(obj, false);
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
