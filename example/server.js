/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*-
 *
 * Example server to compile block intermediate language to koov
 * binary (in intel-hex format).
 *
 * 1) Set path to Aruduino.app to ARDUINO_APP environment variable.
 *
 * 2) Invoke server as follows:
 *   $ cd app/modules/biltrans
 *   $ node example/server.js
 */

'use strict';

let http = require('http');
const PORT = 8234;
const ALLOWED_ORIGIN = 'http://127.0.0.1:3020';

let server = http.createServer((request, response) => {
  response.setHeader('Access-Control-Allow-Origin', ALLOWED_ORIGIN);
  response.setHeader('Access-Control-Request-Method', 'GET,POST,OPTIONS');
  response.setHeader('Access-Control-Allow-Credentials', 'true');

  let error = (code) => {
    response.writeHead(code, {'Content-Type': 'text/plain'});
    response.end();
    request.connection.destroy();
  };

  if (request.method !== 'POST') { return error(400); }

  let body = '';
  request.on('data', (data) => {
    body += data;
    if (body.length > 1e6) { return error(413); }
  });
  request.on('end', () => {
    const biltrans = require('../biltrans.js');
    const scripts = JSON.parse(body);
    let trans = biltrans.translator(scripts);

    const spawn = require('child_process').spawn;
    const build = spawn('/bin/sh', ['./scripts/compile.sh']);

    build.stderr.on('data', (data) => {
      console.log(`build stderr: ${data}`);
    });

    let compiled = '';
    build.stdout.on('data', (data) => { compiled += data; });

    build.on('close', (code) => {
      if (code !== 0)
        return error(400);
      response.end(compiled);
    });

    setTimeout(() => {
      if (build.exitCode) return;
      build.stdin.write(trans.translate());
      build.stdin.end();
    }, 50);
  });
});

server.listen(PORT, () => {
  console.log(`Server listening on: http://localhost:${PORT}`);
});
