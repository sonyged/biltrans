;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_isoginchaku.scm  | jq .

(add-load-path "./example")
(use block)

;; isoginchaku 0714 (sony building version)
(define *scripts*
  `((when-green-flag-clicked
     (forever
      (turn-dcmotor-on M2 NORMAL)
      ,@(append-map
         (^(power&wait)
           `((set-dcmotor-power M2 ,(car power&wait))
             (wait ,(cdr power&wait))))
         '((24 . 0.8)
           (23 . 0.5)
           (18 . 0.5)
           (12 . 6)
           (18 . 0.5)
           (23 . 0.5)
           (26 . 0.8)
           (23 . 0.5)
           (18 . 0.5)
           (12 . 6)
           (18 . 0.5)
           (23 . 0.5)
           ))))))
(define *port-mappings*
  '((M2 . V0)))

(block-list->json *scripts* *port-mappings*)
