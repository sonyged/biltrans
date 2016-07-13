;;; -*- indent-tabs-mode: nil -*-
;;; cd ..
;;; gosh ./example/koov_yadokari.scm  | jq .

(add-load-path "./example")
(use block)

;; yadokari 0713
(define *scripts*
  '((when-green-flag-clicked
     (forever
      (servomotor-synchronized-motion
       15
       (set-servomotor-degree D9 30))
      (servomotor-synchronized-motion
       15
       (set-servomotor-degree D9 150))))))
(define *port-mappings*
  '((D9 . V5)))

(block-list->json *scripts* *port-mappings*)
