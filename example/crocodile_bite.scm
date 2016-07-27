;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/crocodile_bite.scm  | jq .

(add-load-path "./example")
(use block)
(use gauche.parseopt)

(define *sensor-op* '(< light-sensor-value 50))
(let-args *argv*
  ((#f "ir" => (^[] (set! *sensor-op* '(> ir-photo-reflector-value 10))))))

(define (sensor-reacted port)
  `(,(car *sensor-op*) (,(cadr *sensor-op*) ,port) ,(caddr *sensor-op*)))

;; crocodile_bite
(define *scripts*
  `((when-green-flag-clicked
     (forever
      (set-servomotor-degree D9 120)
      (turn-led A0 OFF)
      (turn-led A1 OFF)
      (wait 2)

      (if-then
       ,(sensor-reacted 'A3)
       (set-variable-to bite (pick-random 1 3))
       (if-then
        (= (variable-ref bite) 1)
        (repeat
         5
         (turn-led A0 ON)
         (turn-led A1 OFF)
         (wait 0.2)
         (turn-led A0 OFF)
         (turn-led A1 ON)
         (wait 0.2))
        (turn-led A1 ON)
        (call-function OK)
        (set-servomotor-degree D9 150)
        (wait 1)
        (set-servomotor-degree D9 120))

       (if-then
        (= (variable-ref bite) 2)
        (repeat
         5
         (turn-led A0 ON)
         (turn-led A1 OFF)
         (wait 0.2)
         (turn-led A0 OFF)
         (turn-led A1 ON)
         (wait 0.2))
        (call-function OK)
        (repeat
         3
         (set-servomotor-degree D9 140)
         (wait 0.2)
         (set-servomotor-degree D9 150)
         (wait 0.2)))
  
       (if-then
        (= (variable-ref bite) 3)
        (repeat
         5
         (turn-led A0 ON)
         (turn-led A1 OFF)
         (wait 0.2)
         (turn-led A0 OFF)
         (turn-led A1 ON)
         (wait 0.2))
        (turn-led A0 ON)
        (turn-led A1 OFF)
        (call-function NG)
        (repeat
         3
         (set-servomotor-degree D9 140)
         (wait 0.5)
         (set-servomotor-degree D9 180)
         (wait 0.5))))))

    (function
     OK
     (buzzer-on A2 72)
     (wait 0.2)
     (buzzer-off A2)
     (wait 0.2)
     (buzzer-on A2 69)
     (wait 0.2)
     (buzzer-off A2))

    (function
     NG
     (buzzer-on A2 50)
     (wait 0.5)
     (buzzer-off A2))

    (variable bite 0)))

(define *port-mappings*
  '((A0 . V2) (A1 . V3) (A2 . V6) (A3 . K7) (D9 . V9)))

(block-list->json *scripts* *port-mappings*)
