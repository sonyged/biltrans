;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_helicopter.scm  | jq .

(add-load-path "./example")
(use block)

;; dog
(define *scripts*
  `((when-green-flag-clicked
     (forever
      (if-then-else
       (> (ir-photo-reflector-value A0) 30)
       (then
	,@(let loop ((acc '())
		     (params '((9 48)
			       (11 50)
			       (13 52)
			       (17 53)
			       (20 55)
			       (25 57)
			       (30 59)
			       (35 60)
			       (41 62)
			       (53 64)
			       (61 65)
			       (82 67)))
		     (prev 0))
	    (define (emit p)
	      `(if-then
		(and
		 (< ,prev (ir-photo-reflector-value A4))
		 (<  (ir-photo-reflector-value A4) ,(car p)))
		(buzzer-on A5 ,(cadr p))))
	    (cond ((null? params) (reverse acc))
		  (else (loop (cons (emit (car params)) acc)
			      (cdr params)
			      (caar params)))))
	(if-then
	 (< 82 (ir-photo-reflector-value A4))
	 (buzzer-on A5 69)))
       (else
	(buzzer-off A5)))))))

(define *port-mappings*
  '((A0 A0) (A4 K6) (A5 K7)))

(block-list->json *scripts* *port-mappings*)
