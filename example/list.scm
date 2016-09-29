;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/list.scm  | jq .

(add-load-path "./example")
(use block)

;; servomotor
(define *scripts*
  '((when-green-flag-clicked
     (set-variable-to incr 1)
     (turn-led V2 OFF)
     (turn-led V3 OFF)
     (forever
      (wait 0.01)
      (if-then-else
       (equal? (variable-ref incr) 0)
       (then (list-delete lst 0))
       (else (list-add lst 3)))
      (if-then
       (greater-than? (list-length lst) 30)
       (set-variable-to incr 0))
      (if-then
       (equal? (list-length lst) 0)
       (set-variable-to incr 1))
      (call-function led)))
    (list lst)
    (variable incr 0)
    (function
     led
     (if-then-else
       (= (mod (list-length lst) 5) 3)
       (then
        (turn-led V2 ON)
        (turn-led V3 OFF))
       (else
        (turn-led V2 OFF)
        (turn-led V3 ON))))))
(define *port-mappings*
  '())

(block-list->json *scripts* *port-mappings*)
