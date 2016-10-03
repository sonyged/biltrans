;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/all-blocks.scm  | jq .

(add-load-path "./example")
(use block)

(define dcmotor-ports '(V0 V1))
(define digital-ports '(V2 V3 V4 V5 V6 V7 V8 V9))
(define accel-ports '(K0 K1))
(define analog-ports '(K2 K3 K4 K5 K6 K7))
(define button-ports '(A0 A1 A2 A3))

(define (compare-0 v)
  `(if-then (= ,v 0) (turn-led V2 ON)))

;; servomotor
(define *scripts*
  `((when-green-flag-clicked
     ,@(map (^p `(set-servomotor-degree ,p (+ 90 0))) digital-ports)
     ,@(map (^p `(set-dcmotor-power ,p (- 100 0))) dcmotor-ports)
     ,@(append-map (^p (map (^m `(turn-dcmotor-on ,p ,m)) '(NORMAL REVERSE)))
		   dcmotor-ports)
     ,@(append-map (^p (map (^m `(turn-dcmotor-off ,p ,m)) '(COAST BRAKE)))
		   dcmotor-ports)
     ,@(append-map (^p (map (^f `(buzzer-on ,p ,f)) '(48 72 (+ 40 8))))
		   digital-ports)
     ,@(map (^p `(buzzer-off ,p)) digital-ports)
     ,@(append-map (^p (map (^m `(turn-led ,p ,m)) '(ON OFF)))
		   digital-ports)
     (multi-led 0 50 (+ 50 50))
     ,@(append-map (^p (map (^o (compare-0 `(,o ,p)))
			    '(light-sensor-value
			      ir-photo-reflector-value)))
		   analog-ports)
     ,@(append-map
	(^p (map (^d (compare-0 `(3-axis-digital-accelerometer-value ,p, d)))
		 '(x y z)))
	accel-ports)
     ,@(append-map (^p (map (^m `(if-then (button-value ,p ,m))) '(ON OFF)))
		   button-ports)
     ,@(map (^o (compare-0 `(,o (+ 10.0 -3.0))))
	    '(abs sqrt sin cos tan ln log |e^| |10^|))
     (reset-timer)
     (if-then (= (timer) 0) (turn-led V2 OFF))

     (if-then (= (variable-ref incr) 0) (turn-led V2 ON))
     (set-variable-to incr (+ 1 1))
     (change-variable-by incr (* 3 4))

     (if-then
      (and (greater-than? (list-length lst) 30) (= 2 3))
      (list-add lst (+ (+ 2 (round 3.5)) (round (+ 4 5)))))

     (if-then
      (or (list-contains? lst (+ 3 4)) (< 2 3))
      (list-delete incr (mod 4 2)))

     (if-then
      (not (< (list-ref lst (+ 3 4)) 5))
      (list-insert incr (+ 1 0) (pick-random (* 2 3) 50))
      (list-replace incr (+ 1 0) (* 2 3)))

     (forever
      (repeat (+ 2 2) (wait 0.01))
      (wait (+ 0 1))
      (wait-until (button-value A0 ON))
      (repeat-until
       (button-value A0 OFF)
       (wait (+ 0 1))
       (wait (+ 0 1)))
      (servomotor-synchronized-motion
       (+ 30 40)
       (set-servomotor-degree V2 90)
       (set-servomotor-degree V2 180))
      (breakpoint)
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

(block-list->json *scripts* *port-mappings* :allow-conflict #t)
