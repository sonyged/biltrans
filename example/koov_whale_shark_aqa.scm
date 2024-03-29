;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Copyright (c) 2017 Sony Global Education, Inc.
;;;
;;; cd ..
;;; gosh ./example/koov_yadokari.scm  | jq .

(add-load-path "./example")
(use block)

;; koov_whale_shark_aqa 0714 (sony building version)
(define *scripts*
  `((when-green-flag-clicked
     (set-servomotor-degree D9 90)
     (set-servomotor-degree D10 90)
     (forever
      (if-then
       (button-value A0 ON)
       (call-function fb))
      (if-then
       (button-value A1 ON)
       (call-function turn))))
    (function
     fb
     (forever
      (call-function forward)
      (call-function back)))
    (function
     forward
     (set-dcmotor-power M1 40)
     (set-dcmotor-power M2 40)
     (turn-dcmotor-on M1 REVERSE)
     (turn-dcmotor-on M2 REVERSE)
     (repeat
      2
      (call-function body)))
    (function
     back
     (set-dcmotor-power M1 16)
     (set-dcmotor-power M2 16)
     (turn-dcmotor-on M1 NORMAL)
     (turn-dcmotor-on M2 NORMAL)
     (repeat
      2
      (call-function body)))
    (function
     body
     (if-then-else
      (button-value A1 ON)
      (then
       (call-function turn))
      (else
       ,(servomotor-synchronized-motion 1 '((D9 . 80) (D10 . 115)))))
     (if-then-else
      (button-value A1 ON)
      (then
       (call-function turn))
      (else
       ,(servomotor-synchronized-motion 1 '((D9 . 75) (D10 . 65)))))
     (if-then-else
      (button-value A1 ON)
      (then
       (call-function turn))
      (else
       ,(servomotor-synchronized-motion 1 '((D9 . 100) (D10 . 115)))))
     (if-then-else
      (button-value A1 ON)
      (then
       (call-function turn))
      (else
       ,(servomotor-synchronized-motion 1 '((D9 . 100) (D10 . 65))))))
    (function
     turn
     (repeat-until
      (button-value A0 ON)
      (set-dcmotor-power M1 80)
      (set-dcmotor-power M2 30)
      (turn-dcmotor-on M1 REVERSE)
      (turn-dcmotor-on M2 NORMAL)
      (if-then-else
       (button-value A0 ON)
       (then
        (call-function fb))
       (else
        ,(servomotor-synchronized-motion 1 '((D9 . 75) (D10 . 115)))))
      (if-then-else
       (button-value A0 ON)
       (then
        (call-function fb))
       (else
        ,(servomotor-synchronized-motion 1 '((D10 . 65)))))
      (if-then-else
       (button-value A0 ON)
       (then
        (call-function fb))
       (else
        ,(servomotor-synchronized-motion 1 '((D10 . 115)))))
      (if-then-else
       (button-value A0 ON)
       (then
        (call-function turn))
       (else
        ,(servomotor-synchronized-motion 1 '((D10 . 65)))))))))
(define *port-mappings*
  '((M1 . V0) (M2 . V1) (D9 . V2) (D10 . V3)))

(block-list->json *scripts* *port-mappings*)
