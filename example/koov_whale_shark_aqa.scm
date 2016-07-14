;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_yadokari.scm  | jq .

(add-load-path "./example")
(use block)

;; koov_whale_shark_aqa
(define *scripts*
  '((when-green-flag-clicked
     (set-servomotor-degree D9 90)
     (set-servomotor-degree D10 90)
     (forever
      (if-then
       (equal? (button-value A0) 0)
       (call-function fb))
      (if-then
       (equal? (button-value A1) 0)
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
     (set-dcmotor-power M1 40)
     (set-dcmotor-power M2 40)
     (turn-dcmotor-on M1 NORMAL)
     (turn-dcmotor-on M2 NORMAL)
     (repeat
      2
      (call-function body)))
    (function
     body
     (if-then-else
      (equal? (button-value A1) 0)
      (then
       (call-function turn))
      (else
       (servomotor-synchronized-motion
	1
	(set-servomotor-degree D9 80)
	(set-servomotor-degree D10 115))))
     (if-then-else
      (equal? (button-value A1) 0)
      (then
       (call-function turn))
      (else
       (servomotor-synchronized-motion
	1
	(set-servomotor-degree D9 75)
	(set-servomotor-degree D10 65))))
     (if-then-else
      (equal? (button-value A1) 0)
      (then
       (call-function turn))
      (else
       (servomotor-synchronized-motion
	1
	(set-servomotor-degree D9 100)
	(set-servomotor-degree D10 115))))
     (if-then-else
      (equal? (button-value A1) 0)
      (then
       (call-function turn))
      (else
       (servomotor-synchronized-motion
	1
	(set-servomotor-degree D9 100)
	(set-servomotor-degree D10 65)))))
    (function
     turn
     (repeat-until
      (equal? (button-value A0) 0)
      (set-dcmotor-power M1 80)
      (set-dcmotor-power M2 30)
      (turn-dcmotor-on M1 REVERSE)
      (turn-dcmotor-on M2 NORMAL)
      (if-then-else
       (equal? (button-value A0) 0)
       (then
	(call-function fb))
       (else
	(servomotor-synchronized-motion
	 1
	 (set-servomotor-degree D9 75)
	 (set-servomotor-degree D10 115))))
      (if-then-else
       (equal? (button-value A0) 0)
       (then
	(call-function fb))
       (else
	(servomotor-synchronized-motion
	 1
	 (set-servomotor-degree D10 65))))
      (if-then-else
       (equal? (button-value A0) 0)
       (then
	(call-function fb))
       (else
	(servomotor-synchronized-motion
	 1
	 (set-servomotor-degree D10 115))))
      (if-then-else
       (equal? (button-value A0) 0)
       (then
	(call-function turn))
       (else
	(servomotor-synchronized-motion
	 1
	 (set-servomotor-degree D10 65))))))))
(define *port-mappings*
  '((M1 . V0) (M2 . V1) (D9 . V2) (D10 . V3)))

(block-list->json *scripts* *port-mappings*)
