;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Copyright (c) 2017 Sony Global Education, Inc.
;;;
;;; cd ..
;;; gosh ./example/koov_helicopter.scm  | jq .

(add-load-path "./example")
(use block)

;; helicopter 0719
(define *scripts*
  `((when-green-flag-clicked
     (set-dcmotor-power M1 80)
     (turn-dcmotor-on M1 NORMAL)
     (set-servomotor-degree D9 90)
     (forever
      (if-then
       (button-value A0 ON)
       ,(servomotor-synchronized-motion 4 '((D9 . 90))))
      (if-then
       (button-value A1 ON)
       ,(servomotor-synchronized-motion 3 '((D9 . 130))))
      (if-then
       (button-value A2 ON)
       ,(servomotor-synchronized-motion 3 '((D9 . 50))))))))

(define *port-mappings*
  '((M1 . V0) (D9 . V2) (A0 . A0)  (A1 . A1)  (A2 . A2)))

(block-list->json *scripts* *port-mappings*)
