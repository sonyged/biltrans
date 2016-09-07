;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/buzzer-onshot.scm  | jq .

(add-load-path "./example")
(use block)

;; servomotor
(define *scripts*
  '((when-green-flag-clicked
     (buzzer-on V2 60)
     (wait 1)
     (buzzer-off V2))))
(define *port-mappings*
  '())

(block-list->json *scripts* *port-mappings*)
