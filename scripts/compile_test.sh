#!/bin/sh

# Set basic authentication information to USERPASS environment variable.
# export USERPASS=user:pass
#
# Total 10 jobs, max 10 jobs
# seq 10 | parallel -j 10 -n 1 sh scripts/compile_test.sh
#
# Total 10 jobs, max 4 jobs
# seq 10 | parallel -j 4 -n 1 sh scripts/compile_test.sh
#

HOST=www-stg.koov.io
SCRIPT='
{
  "scripts": [
    {
      "name": "when-green-flag-clicked",
      "blocks": [
        {
          "name": "repeat",
          "count": 10,
          "blocks": [
            {
              "name": "set-variable-to",
              "variable": "ir-photo-reflector",
              "value": {
                "name": "ir-photo-reflector-value",
                "port": "K7"
              }
            },
            {
              "name": "turn-led",
              "port": "V2",
              "mode": "ON"
            },
            {
              "name": "wait",
              "secs": 1
            },
            {
              "name": "turn-led",
              "port": "V2",
              "mode": "OFF"
            },
            {
              "name": "wait",
              "secs": 1
            }
          ]
        }
      ]
    },
    {
      "name": "variable",
      "variable": "ir-photo-reflector",
      "value": 0
    }
  ],
  "port-settings": {
    "A0": "push-button",
    "A1": "push-button",
    "V2": "led",
    "K6": "light-sensor",
    "K7": "ir-photo-reflector"
  }
}
'

ENCODED_SCRIPT=$(node -e "
    console.log(JSON.stringify(JSON.stringify(${SCRIPT})))
")

exec curl -v \
    -u ${USERPASS} \
    -o /dev/null \
    -X POST \
    -d '{ "program": '"${ENCODED_SCRIPT}"' }' \
    -H 'Content-Type: application/json' \
    https://$HOST/compile.json
