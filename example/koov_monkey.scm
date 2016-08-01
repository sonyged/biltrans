;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_helicopter.scm  | jq .

(add-load-path "./example")
(use block)

;; dog
(define *scripts*
  `((when-green-flag-clicked
     (set-servomotor-degree D9 10)
     (set-servomotor-degree D10 170)
     (forever
      ,(servomotor-synchronized-motion 15 '((D9 10) (D10 170)))
      (wait 0.1)
      ,(servomotor-synchronized-motion 20 '((D9 90) (D10 90)))
      (wait 0.1)))))

(define *port-mappings*
  '((D9 V2) (D10 V3)))

(block-list->json *scripts* *port-mappings*)
