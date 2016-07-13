;;;
;;; cd ..
;;; gosh ./example/koov_isoginchaku.scm  | jq .

(add-load-path "./example")
(use block)

;; isoginchaku 0713
(define *scripts*
  `((when-green-flag-clicked
     (forever
      (turn-dcmotor-on M2 NORMAL)
      ,@(append-map
	 (^(power&wait)
	   `((set-dcmotor-power M2 ,(car power&wait))
	     (wait ,(cdr power&wait))))
	 '((40 . 1)
	   (35 . 0.5)
	   (30 . 0.5)
	   (20 . 6)
	   (30 . 0.5)
	   (35 . 0.5)
	   (50 . 1)
	   (35 . 0.5)
	   (30 . 0.5)
	   (20 . 6)
	   (30 . 0.5)
	   (35 . 0.5)
	   ))))))
(define *port-mappings*
  '((M2 . V0)))

(block-list->json *scripts* *port-mappings*)
