;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Copyright (c) 2017 Sony Global Education, Inc.
;;;
;;; cd ..
;;; gosh ./example/timer.scm  | jq .

(add-load-path "./example")
(use block)

;; servomotor
(define *scripts*
  '((when-green-flag-clicked
     (reset-timer)
     (turn-led V2 OFF)
     (turn-led V3 OFF)
     (forever
      (wait 0.01)
      (if-then-else
       (= (mod (timer) 5) 3)
       (then
        (turn-led V2 ON)
        (turn-led V3 OFF))
       (else
        (turn-led V2 OFF)
        (turn-led V3 ON)))))))
(define *port-mappings*
  '())

(block-list->json *scripts* *port-mappings*)
