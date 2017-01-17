;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Copyright (c) 2017 Sony Global Education, Inc.
;;;
;;; cd ..
;;; gosh ./example/koov_s_armcar_touchsensor_0806.scm  | jq .

(add-load-path "./example")
(use block)

;; s_armcar_touchsensor 0806
(define *scripts*
  `((when-green-flag-clicked
     (set-servomotor-degree D11 160)
     (set-servomotor-degree D9 20)
     (if-then
      (button-value A1 ON)
      (call-function "photo"))
     (if-then
      (button-value A3 ON)
      (call-function "no_photo")))
    (function
     photo
     (set-servomotor-degree D11 160)
     (set-servomotor-degree D9 20)
     (forever
      (if-then
       (> (ir-photo-reflector-value A2) 35)
       (wait 1)
       ,(servomotor-synchronized-motion
	 5
	 '((D9 160) (D11 20)))
       (wait 1)
       (forever
	(set-dcmotor-power M1 70)
	(set-dcmotor-power M2 30)
	(turn-dcmotor-on M1 REVERSE)
	(turn-dcmotor-on M2 REVERSE)
	(if-then
	 (touch-sensor-value A0 ON)
	 (call-function touch))))))
    (function
     no_photo
     (wait 1)
     ,(servomotor-synchronized-motion
       5
       '((D9 160) (D11 20)))
     (forever
      (set-dcmotor-power M1 70)
      (set-dcmotor-power M2 40)
      (turn-dcmotor-on M1 REVERSE)
      (turn-dcmotor-on M2 REVERSE)
      (if-then
       (touch-sensor-value A0 ON)
       (call-function touch))))
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
  '((M1 V0) (M2 V1) (D9 V2) (D11 V3) (A0 K2) (A2 K4) (A1 A1) (A3 A3)))

(block-list->json *scripts* *port-mappings*)
