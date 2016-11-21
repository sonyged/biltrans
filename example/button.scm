;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/button.scm  | jq .

(add-load-path "./example")
(use block)

;; servomotor
(define *scripts*
  '((when-green-flag-clicked
     (turn-led V2 ON)
     (turn-led V3 ON)
     (wait 1)
     (turn-led V2 OFF)
     (turn-led V3 OFF)
     (forever
      (turn-led V3 ON)
      (wait 0.2)
      (if-then-else
       (button-value A0 ON)
       (then
        (turn-led V2 ON)
        (wait 0.5))
       (else
        (turn-led V2 OFF)
        (wait 0.5)))
      (turn-led V3 OFF)
      (wait 0.2)))))
(define *port-mappings*
  '())

(block-list->json *scripts* *port-mappings*)
