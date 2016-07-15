/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 */

'use strict';
const fs = require('fs');
const path = require('path');

/*
 * Mapping from stduino to koov:
 *
 * LED Red: A0 -> V2
 * LED Green: A1 -> V3
 * Buzzer: A2 -> V6
 * Servo motor: D9 -> V9
 * Sensor: A3 -> K7
 */

const module_directory = path.dirname(module.filename);
const json = fs.readFileSync(`${module_directory}/crocodile_bite.json`);
const scripts = JSON.parse(json);

module.exports = {
  scripts: scripts
};
