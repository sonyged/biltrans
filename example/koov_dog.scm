;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Copyright (c) 2017 Sony Global Education, Inc.
;;;
;;; cd ..
;;; gosh ./example/koov_helicopter.scm  | jq .

(add-load-path "./example")
(use block)

;; dog
(define *scripts*
  `((when-green-flag-clicked
     (set-servomotor-degree D9 90)
     (set-servomotor-degree D10 120)
     (set-servomotor-degree D11 60)
     (forever
      ,(servomotor-synchronized-motion 10 '((D9 90) (D10 120) (D11 60)))
      ,(servomotor-synchronized-motion 10 '((D9 120) (D10 90) (D11 60)))
      ,(servomotor-synchronized-motion 10 '((D9 90) (D10 60) (D11 120)))
      ,(servomotor-synchronized-motion 10 '((D9 60) (D10 90) (D11 120)))))))

(define *port-mappings*
  '((D9 V2) (D10 V3) (D11 V4)))

(block-list->json *scripts* *port-mappings*)
