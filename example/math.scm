;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/math.scm  | jq .

(add-load-path "./example")
(use block)

;; servomotor
(define *scripts*
  '((when-green-flag-clicked
     (reset-timer)
     (turn-led V2 OFF)
     (turn-led V3 OFF)
     (forever
      (wait 0.01)
      (if-then-else
       (and (= (abs 5) 5)
	    (= (abs -5) 5)
	    (= (sqrt 9) 3)
	    (= (sqrt 16) 4)
	    (= (sin 0) 0)
	    (= (sin (/ 180 2)) 1)
	    (= (cos 0) 1)
	    (= (round (* 10000000 (cos (/ 180 2)))) 0)
	    (= (tan 0) 0)
	    (= (tan (/ 180 4)) 1)
	    (= (ln 1) 0)
	    (= (ln 2.71828182846) 1)
	    (= (log 1) 0)
	    (= (log 10) 1)
	    (= (e^ 0) 1)
	    (= (round (* 10000000 (e^ 1))) (round (* 10000000 2.71828182846))))
       (then
        (turn-led V2 ON)
        (turn-led V3 OFF))
       (else
        (turn-led V2 OFF)
        (turn-led V3 ON)))))))
(define *port-mappings*
  '())

(block-list->json *scripts* *port-mappings*)
