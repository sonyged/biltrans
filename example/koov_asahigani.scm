;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_asahigani.scm  | jq .

(add-load-path "./example")
(use block)

;;; koov_asahigani_0706.bpd
(define *scripts*
  `((when-green-flag-clicked
     ,@(map (^[args] `(set-servomotor-degree ,@args))
            '((D2 90) (D4 90) (D9 90) (D10 90) (D11 90) (D12 90)))
     (forever
      ,(servomotor-synchronized-motion
        10
        '((D2 90) (D4 90) (D9 90) (D10 90) (D11 90) (D12 90)))
      (if-then
       (touch-sensor-value A0 ON)
       (wait 0.5)
       (if-then
        (touch-sensor-value A1 OFF)
        (call-function walk01))
       (if-then
        (touch-sensor-value A1 ON)
        (call-function walk02)))
      (if-then
       (and (touch-sensor-value A0 OFF) (touch-sensor-value A1 ON))
       (call-function eat))))
    (function
     eat
     (repeat-until
      (touch-sensor-value A1 OFF)
      ,(servomotor-synchronized-motion 8 '((D2 125) (D4 90)))
      ,(servomotor-synchronized-motion 10 '((D2 90) (D4 90)))
      ,(servomotor-synchronized-motion 8 '((D2 90) (D4 125)))
      ,(servomotor-synchronized-motion 10 '((D2 90) (D4 90)))))
    (function
     walk01
     (repeat-until
      (touch-sensor-value A0 OFF)
      ,(servomotor-synchronized-motion
        14
        '((D2 95) (D4 105) (D9 75) (D10 65) (D11 105) (D12 115)))
      ,(servomotor-synchronized-motion
        14
        '((D2 105) (D4 95) (D9 115) (D10 105) (D11 65) (D12 75)))))
    (function
     walk02
     (repeat-until
      (and (touch-sensor-value A0 OFF) (touch-sensor-value A1 OFF))
      ,(servomotor-synchronized-motion
        14
        '((D2 120) (D4 120) (D9 90) (D10 75) (D11 105) (D12 105)))
      ,(servomotor-synchronized-motion
        14
        '((D2 90) (D4 90) (D9 105) (D10 90) (D11 75) (D12 75)))))
    ))

(define *port-mappings*
  '((A0 . K6) (A1 . K7)
    (D2 . V2) (D4 . V3)
    (D9 . V5) (D10 . V6) (D11 . V7) (D12 . V8)))

(block-list->json *scripts* *port-mappings*)
