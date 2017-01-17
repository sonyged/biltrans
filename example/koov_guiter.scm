;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Copyright (c) 2017 Sony Global Education, Inc.
;;;
;;; cd ..
;;; gosh ./example/koov_helicopter.scm  | jq .

(add-load-path "./example")
(use block)

(define *fret*
  ;; list of ir-photo-reflector value and buzzer pitch.
  '((10 57)
    (11 59)
    (13 60)
    (15 62)
    (19 64)
    (24 65)
    (32 67)
    (43 69)
    (59 71)
    (#f 72)))

;; guiter
(define *scripts*
  `((when-green-flag-clicked
     (forever
      (if-then-else
       (> (ir-photo-reflector-value A0) 30)
       (then
        ,@(let loop ((acc '())
                     (params *fret*)
                     (prev 1))
            (define (emit p)
              `(if-then
                ,(if (null? (cdr params))
                     `(< ,(- prev 1) (ir-photo-reflector-value A4))
                     `(and
                       (< ,(- prev 1) (ir-photo-reflector-value A4))
                       (<  (ir-photo-reflector-value A4) ,(car p))))
                (buzzer-on A5 ,(cadr p))))
            (cond ((null? params) (reverse acc))
                  (else (loop (cons (emit (car params)) acc)
                              (cdr params)
                              (caar params))))))
       (else
        (buzzer-off A5)))))))

(define *port-mappings*
  '((A0 K4) (A4 K6) (A5 V6)))

(block-list->json *scripts* *port-mappings*)
