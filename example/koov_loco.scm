;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_loco.scm  | jq .

(add-load-path "./example")
(use block)

;; loco 0719
(define *scripts*
  '((when-green-flag-clicked
     (set-dcmotor-power M1 60)
     (set-dcmotor-power M2 60)
     (wait 1)
     (forever
      (if-then-else
       (< (ir-photo-reflector-value A0) 45)
       (then
	(turn-dcmotor-on M1 NORMAL)
	(turn-dcmotor-off M2 COAST))
       (else
	(turn-dcmotor-off M1 COAST)
	(turn-dcmotor-on M2 NORMAL)))))))
(define *port-mappings*
  '((M1 . V0) (M2 . V1) (A0 . K2)))

(block-list->json *scripts* *port-mappings*)
