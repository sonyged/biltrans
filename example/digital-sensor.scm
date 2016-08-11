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
      ;; old-style
      (if-then-else
       (= (button-value A0) 0)
       (then
        (turn-led V2 ON))
       (else
        (turn-led V2 OFF)))
      (if-then-else
       (= (touch-sensor-value K6) 1)
       (then
        (turn-led V3 OFF))
       (else
        (turn-led V3 ON)))
      (wait 0.5)
      ;; new-style
      (if-then-else
       (button-value A0 ON)
       (then
        (turn-led V2 ON))
       (else
        (turn-led V2 OFF)))
      (if-then-else
       (touch-sensor-value K6 OFF)
       (then
        (turn-led V3 OFF))
       (else
        (turn-led V3 ON)))
      (wait 0.5)))))
(define *port-mappings*
  '())

(block-list->json *scripts* *port-mappings*)
