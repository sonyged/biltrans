;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_asahigani.scm  | jq .

(add-load-path "./example")
(use block)

;;; koov_asahigani_0706.bpd
(define *scripts*
  '((when-green-flag-clicked
     (forever
      (servomotor-synchronized-motion
       10
       (set-servomotor-degree D2 90)
       (set-servomotor-degree D4 90)
       (set-servomotor-degree D9 90)
       (set-servomotor-degree D10 90)
       (set-servomotor-degree D11 90)
       (set-servomotor-degree D12 90))
      (if-then
       (= (touch-sensor-value A0) 0)
       (wait 0.5)
       (if-then
	(= (touch-sensor-value A1) 1)
	(call-function walk01))
       (if-then
	(= (touch-sensor-value A1) 0)
	(call-function walk02)))
      (if-then
       (and (= (touch-sensor-value A0) 1) (= (touch-sensor-value A1) 0))
       (call-function eat))))
    (function
     eat
     (repeat-until
      (= (touch-sensor-value A1) 1)
      (servomotor-synchronized-motion
       8
       (set-servomotor-degree D2 125)
       (set-servomotor-degree D4 90))
      (servomotor-synchronized-motion
       10
       (set-servomotor-degree D2 90)
       (set-servomotor-degree D4 90))
      (servomotor-synchronized-motion
       8
       (set-servomotor-degree D2 90)
       (set-servomotor-degree D4 125))
      (servomotor-synchronized-motion
       10
       (set-servomotor-degree D2 90)
       (set-servomotor-degree D4 90))))
    (function
     walk01
     (repeat-until
      (= (touch-sensor-value A0) 1)
      (servomotor-synchronized-motion
       14
       (set-servomotor-degree D2 95)
       (set-servomotor-degree D4 105)
       (set-servomotor-degree D9 75)
       (set-servomotor-degree D10 65)
       (set-servomotor-degree D11 105)
       (set-servomotor-degree D12 115))
      (servomotor-synchronized-motion
       14
       (set-servomotor-degree D2 105)
       (set-servomotor-degree D4 95)
       (set-servomotor-degree D9 115)
       (set-servomotor-degree D10 105)
       (set-servomotor-degree D11 65)
       (set-servomotor-degree D12 75))))
    (function
     walk02
     (repeat-until
      (and (= (touch-sensor-value A0) 1) (= (touch-sensor-value A1) 1))
      (servomotor-synchronized-motion
       14
       (set-servomotor-degree D2 120)
       (set-servomotor-degree D4 120)
       (set-servomotor-degree D9 90)
       (set-servomotor-degree D10 75)
       (set-servomotor-degree D11 105)
       (set-servomotor-degree D12 105))
      (servomotor-synchronized-motion
       14
       (set-servomotor-degree D2 90)
       (set-servomotor-degree D4 90)
       (set-servomotor-degree D9 105)
       (set-servomotor-degree D10 90)
       (set-servomotor-degree D11 75)
       (set-servomotor-degree D12 75))))
    ))
(define *port-mappings*
  '((A0 . K6) (A1 . K7)
    (D2 . V2) (D4 . V3)
    (D9 . V5) (D10 . V6) (D11 . V7) (D12 . V8)))

(block-list->json *scripts* *port-mappings*)
