;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_s_armcar_touchsensor.scm  | jq .

(add-load-path "./example")
(use block)

;; s_armcar_touchsensor 0726
(define *scripts*
  `((when-green-flag-clicked
     (set-servomotor-degree D11 160)
     (set-servomotor-degree D9 20)
     (forever
      (if-then
       (> (ir-photo-reflector-value A2) 20)
       (wait 1)
       ,(servomotor-synchronized-motion
	 5
	 '((D9 160) (D11 20)))
       (wait 1)
       (forever
	(set-dcmotor-power M1 70)
	(set-dcmotor-power M2 30)
	(turn-dcmotor-on M1 NORMAL)
	(turn-dcmotor-on M2 NORMAL)
	(if-then
	 (= (touch-sensor-value A0) 0)
	 (call-function touch))))))
    (function
     touch
     (turn-dcmotor-off M1 BRAKE)
     (turn-dcmotor-off M2 BRAKE)
     (wait 1)
     ,(servomotor-synchronized-motion
       5
       '((D11 160) (D9 20)))
     (wait-until (< (ir-photo-reflector-value A2) 10))
     (wait 1)
     (wait-until (> (ir-photo-reflector-value A2) 20))
     (wait 1)
     ,(servomotor-synchronized-motion
       5
       '((D9 160) (D11 20))))))
(define *port-mappings*
  '((M1 V0) (M2 V1) (D9 V2) (D11 V3) (A0 K6) (A2 K7)))

(block-list->json *scripts* *port-mappings*)
