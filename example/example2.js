/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 */

'use strict';

const scripts_led = {
  scripts: [
    {
      name: 'when-green-flag-clicked',
      blocks: [
        {
          name: 'repeat',
          count: 10,
          blocks: [
            { name: 'turn-led', port: 'A1', mode: 'ON' },
            { name: 'wait', secs: 1 },
            { name: 'turn-led', port: 'A1', mode: 'OFF' },
            { name: 'wait', secs: 1 }
          ]
        }
      ]
    }
  ]
};

const scripts_buzzer = {
  scripts: [
    {
      name: 'when-green-flag-clicked',
      blocks: [
        {
          name: 'repeat',
          count: 10,
          blocks: [
            { name: 'buzzer-on', port: 'A2', mode: 'ON',
	      frequency: 72 },
            { name: 'wait', secs: 1 },
            { name: 'buzzer-on', port: 'A2', mode: 'ON',
	      frequency: 69 },
            { name: 'wait', secs: 1 },
            { name: 'buzzer-off', port: 'A2', mode: 'OFF' },
            { name: 'wait', secs: 1 }
          ]
        }
      ]
    }
  ]
};

const scripts_sensor = {
  scripts: [
    {
      name: 'when-green-flag-clicked',
      blocks: [
	{
	  name: 'repeat',
	  count: 10,
	  blocks: [
	    {
	      name: 'if-then-else',
	      condition: {
		name: 'less-than?',
		x: { name: 'light-sensor-value', port: 'A3' },
		y: 128
	      },
	      'then-blocks': [
		{ name: 'turn-led', port: 'A0', mode: 'ON' },
		{ name: 'turn-led', port: 'A1', mode: 'OFF' },
	      ],
	      'else-blocks': [
		{ name: 'turn-led', port: 'A0', mode: 'OFF' },
		{ name: 'turn-led', port: 'A1', mode: 'ON' },
	      ]
	    },
          ]
        }
      ]
    }
  ]
};

const scripts_servo = {
  scripts: [
    {
      name: 'when-green-flag-clicked',
      blocks: [
	{
	  name: 'repeat',
	  count: 10,
	  blocks: [
            { name: 'turn-led', port: 'A1', mode: 'ON' },
            { name: 'wait', secs: 1 },
            { name: 'turn-led', port: 'A1', mode: 'OFF' },
            { name: 'wait', secs: 1 },
	    {
	      name: 'if-then-else',
	      condition: {
		name: 'less-than?',
		x: { name: 'light-sensor-value', port: 'A3' },
		y: 128
	      },
	      'then-blocks': [
		{ name: 'set-servomotor-degree', port: 'D9', degree: 90 },
	      ],
	      'else-blocks': [
		{ name: 'set-servomotor-degree', port: 'D9', degree: 180 },
	      ]
	    },
          ]
        }
      ]
    }
  ]
};

module.exports = {
  scripts: scripts_servo
};
