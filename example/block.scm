;;; -*- indent-tabs-mode: nil -*-
;;;
;;; Convert block intermediate language written in S-expression to JSON.
;;;

(define-module block
  (use rfc.json)
  (use util.match)
  (use gauche.collection)
  (use srfi-13)
  (export block-list->json))
(select-module block)

;;; Hat blocks are the block that start every script.
;;;   when-green-flag-clicked BLOCKS
;;;   when-key-pressed KEY BLOCKS
;;;   when-this-sprite-clicked BLOCKS
;;;   when-i-receive EVENT BLOCKS
;;;   when-backdrop-switches-to ARG BLOCKS
;;;   when-i-start-as-a-clone BLOCKS
;;;   when-greater-than LHS RHS BLOCKS
;;;   when CONDITION BLOCKS
;;;   when-tilt-is ARG BLOCKS
;;;   when-distance-smaller-than ARG BLOCKS
;;;   when-x-op-y X OP Y blocks
;;;
;;; Stack blocks are the block that perform main command.
;;;   move-steps ARG
;;;   turn-cw DEGREES
;;;   turn-ccw DEGREES
;;;   wait SECS
;;;   turn-led PORT MODE
;;;     :
;;;
;;; Boolean blocks are the conditions.
;;;   touching? ARG
;;;   touching-color? ARG
;;;   color-is-touching? COLOR ARG
;;;   mouse-down?
;;;   key-pressed? KEY
;;;   less-than? X Y
;;;   equal? X Y
;;;   greater-than? X Y
;;;   and? X Y
;;;   or? X Y
;;;   not ARG
;;;   contains? X Y
;;;
;;; Reporter blocks are the values.
;;;   x-position
;;;   y-position
;;;   direction
;;;   loudness
;;;   + X Y
;;;   - X Y
;;;   * X Y
;;;   / X Y
;;;      :
;;;
;;; C blocks are blocks that take the shape of "C's".
;;;   forever BLOCKS
;;;   repeat COUNT BLOCKS
;;;   if-then CONDITION BLOCKS
;;;   if-then-else CONDITION (then THEN-BLOCKS) (else ELSE-BLOCKS)
;;;   repeat-until CONDITION BLOCKS
;;;
;;; Cap blocks are the blocks that ends scripts.
;;;   stop ARG
;;;   delete-this-clone

;;; Data blocks
;;;   Variable block
;;;   List block

;;; Studuino
;;
;;; Motion
;;;   set-servomotor-degree PORT DEGREE          ;; DEGREE: integer
;;;   set-dcmotor-power PORT POWER               ;; POWER: integer
;;;   turn-dcmotor-on PORT DIRECTION             ;; DIRECTION: normal | reverse
;;;   turn-dcmotor-off PORT MODE                 ;; MODE: brake | coast
;;;   buzzer-on PORT FREQ                        ;; FREQ: integer
;;;   buzzer-off PORT
;;;   turn-led PORT MODE                         ;; MODE: on | off
;;;
;;; Sensing
;;;   light-sensor-value PORT
;;;   touch-sensor-value PORT
;;;   sound-sensor-value PORT
;;;   ir-photo-reflector-value PORT
;;;   3-axis-digital-accelerometer-value PORT
;;;   button-value PORT
;;;   reset-timer
;;;   timer
;;;
;;; Control
;;;   function FUNC
;;;   call-function FUNC
;;;   wait-secs SECS
;;;   forever BLOCKS
;;;   repeat COUNT BLOCKS
;;;   forever-if CONDITION BLOCKS
;;;   if-then CONDTION BLOCKS
;;;   if-then-else CONDITION (then THEN-BLOCKS) (else ELSE-BLOCKS)
;;;   wait-until CONDITION
;;;   repeat-until CONDITION BLOCKS
;;;   servomotor-synchronized-motion SPEED BLOCKS      ;; SPEED: integer
;;;
;;; Operators
;;;   + X Y
;;;   - X Y
;;;   * X Y
;;;   / X Y
;;;   pick-random FROM TO
;;;   < X Y
;;;   = X Y
;;;   > X Y
;;;   and X Y
;;;   or X Y
;;;   not X
;;;   mod X Y
;;;   round X
;;;   math OP X      ;; OP: abs | sqrt | sin | cos | tan | ln | log | e^ | 10^
;;;
;;; Data blocks
;;;  Variable block
;;;   set-varibale-to VARIABLE VALUE
;;;   change-varibale-by VARIABLE VALUE
;;;   variable-ref VARIABLE
;;;  List block
;;;   list-add LIST VALUE
;;;   list-delete LIST POSITION
;;;   list-insert LIST POSITION VALUE
;;;   list-replace LIST POSITION VALUE
;;;   list-ref LIST POSITION
;;;   list-length LIST
;;;   list-contains? LIST VALUE

(define (map-ports mapping script)
  (cond ((pair? script)
  (cons (map-ports mapping (car script))
        (map-ports mapping (cdr script))))
 ((assq script mapping) => cdr)
 (else script)))

(define (map-op op)
  (case op
    ((=) 'equal?)
    ((<) 'less-than?)
    ((>) 'greater-than?)
    (else op)))
(define (binary-op? op)
  (memq (map-op op) '(and or less-than? greater-than? equal?)))
(define (turn-op? op)
  (memq op '(turn-cw turn-ccw)))
(define (change-op? op)
  (memq op '(change-by change-effect-by)))
(define (sensor-op? op)
  (memq op '(light-sensor-value
             button-value
             3-axis-digital-accelerometer-value
             ir-photo-reflector-value
             sound-sensor-value
             touch-sensor-value)))

;; (get 'if 'lisp-indent-function)
;; (put 'match 'scheme-indent-function 1)
;; (put 'let1 'scheme-indent-function 2)
;; (put 'unless 'scheme-indent-function 1)
;; (put 'when 'scheme-indent-function 1)
(define (jsonfy e)
  (match e
    [('when-green-flag-clicked blocks ...)
     `((name . "when-green-flag-clicked")
       (blocks . ,(blocks->json blocks)))]
    [('when-key-pressed key blocks ...)
     `((name . "when-key-pressed")
       (key . ,key)
       (blocks . ,(blocks->json blocks)))]
    [('when-i-receive event blocks ...)
     `((name . "when-i-receive")
       (event . ,event)
       (blocks . ,(blocks->json blocks)))]
    [('forever blocks ...)
     `((name . "forever")
       (blocks . ,(blocks->json blocks)))]
    [('repeat count blocks ...)
     `((name . "repeat")
       (count . ,(jsonfy count))
       (blocks . ,(blocks->json blocks)))]
    [('repeat-until condition blocks ...)
     `((name . "repeat-until")
       (condition . ,(jsonfy condition))
       (blocks . ,(blocks->json blocks)))]
    [('if-then condition then ...)
     `((name . "if-then")
       (condition . ,(jsonfy condition))
       (blocks . ,(blocks->json then)))]
    [('if-then-else condition ('then then ...) ('else else ...))
     `((name . "if-then-else")
       (condition . ,(jsonfy condition))
       (then-blocks . ,(blocks->json then))
       (else-blocks . ,(blocks->json else)))]
    [('key-pressed? key)
     `((name . "key-pressed?")
       (key . ,key))]
    [((? binary-op? op) x y)
     `((name . ,(symbol->string (map-op op)))
       (x . ,(jsonfy x))
       (y . ,(jsonfy y)))]
    [((? turn-op? op) degrees)
     `((name . ,(symbol->string op))
       (degrees . ,(jsonfy degrees)))]
    [((? change-op? op) what amount)
     `((name . ,(symbol->string op))
       (what . ,(jsonfy what))
       (amount . ,(jsonfy amount)))]
    [((? sensor-op? op) port)
     `((name . ,(symbol->string op))
       (port . ,(jsonfy port)))]
    [(`turn-led port mode)
     `((name . "turn-led")
       (port . ,(jsonfy port))
       (mode . ,(jsonfy mode)))]
    [(`buzzer-on port frequency)
     `((name . "buzzer-on")
       (port . ,(jsonfy port))
       (frequency . ,(jsonfy frequency)))]
    [(`buzzer-off port)
     `((name . "buzzer-off")
       (port . ,(jsonfy port)))]
    [(`servomotor-synchronized-motion speed blocks ...)
     `((name . "servomotor-synchronized-motion")
       (speed . ,(jsonfy speed))
       (blocks . ,(blocks->json blocks)))]
    [(`set-servomotor-degree port degree)
     `((name . "set-servomotor-degree")
       (port . ,(jsonfy port))
       (degree . ,(jsonfy degree)))]
    [(`set-dcmotor-power port power)
     `((name . "set-dcmotor-power")
       (port . ,(jsonfy port))
       (power . ,(jsonfy power)))]
    [(`turn-dcmotor-on port direction)
     `((name . "turn-dcmotor-on")
       (port . ,(jsonfy port))
       (direction . ,(jsonfy direction)))]
    [(`turn-dcmotor-off port mode)
     `((name . "turn-dcmotor-off")
       (port . ,(jsonfy port))
       (mode . ,(jsonfy mode)))]
    [(`set-variable-to variable value)
     `((name . "set-variable-to")
       (variable . ,(jsonfy variable))
       (value . ,(jsonfy value)))]
    [(`variable variable value)
     `((name . "variable")
       (variable . ,(jsonfy variable))
       (value . ,(jsonfy value)))]
    [(`variable-ref variable)
     `((name . "variable-ref")
       (variable . ,(jsonfy variable)))]
    [(`call-function function)
     `((name . "call-function")
       (function . ,(jsonfy function)))]
    [(`function function blocks ...)
     `((name . "function")
       (function . ,(jsonfy function))
       (blocks . ,(blocks->json blocks)))]
    [(`pick-random from to)
     `((name . "pick-random")
       (from . ,(jsonfy from))
       (to . ,(jsonfy to)))]
    [('loudness)
     '((name . "loudness"))]
    [('stop what)
     `((name . "stop")
       (what . ,what))]
    [('wait secs)
     `((name . "wait")
       (secs . ,secs))]
    [(? number? x) x]
    [(? string? x) x]
    [(? symbol? x) (symbol->string x)]
    (() ())))

(define (blocks->json blocks)
  (list->vector (map jsonfy blocks)))

(define (generate-c json)
  (define initializers '())
  (define ports '())
  (define thread-counter 0)

  ;; accessors
  (define (block-attr attr block)
    (cond ((assq attr block) => cdr)
          (else (errorf "no ~a found ~a" attr block))))
  (define (block-name block)
    (string->symbol (block-attr 'name block)))

  ;; indent code by 2 columns
  (define (indent code)
    (map (^c #"  ~c") code))
  ;; convert blocks inside this block to code.
  (define (block->code block)
    (let1 blocks (block-attr 'blocks block)
      (rlet1 code (append-map emit-block (vector->list blocks)))))

  ;; emit C code for each block
  (define (forever block)
    (cons "while (1) {\n" (append (indent (block->code block)) (list "}\n"))))
  (define (when-green-flag-clicked block)
    (inc! thread-counter)
    (cons (format "NIL_WORKING_AREA(waThread~a, 16);\n\
                   NIL_THREAD(Thread~a, arg) {\n"
                  thread-counter thread-counter)
          (append (indent (block->code block)) (list "}\n"))))
  (define (turn-led block)
    (let ((port (block-attr 'port block))
          (mode (block-attr 'mode block)))
      (unless (member port ports)
        (push! ports port))
      (format "board.LED(PORT_~a, ~a);\n" port mode)))
  (define (wait block)
    (let ((secs (block-attr 'secs block)))
      (format "nilThdSleepMilliseconds(~a);\n" (x->integer (* secs 1000)))))

  ;; emit C code for initializers
  (define (emit-initializers)
    (let1 v '()
      (for-each (^p (push! v p))
                '("#include <Arduino.h>\n"
                  "#include <Servo.h>\n"
                  "#include <Wire.h>\n"
                  "#include <MMA8653.h>\n"
                  "#include <MPU6050.h>\n"
                  "#include <IRremoteForStuduino.h>\n"
                  "#include <ColorSensor.h>\n"
                  "#include \"Studuino.h\"\n"
                  "#include <NilRTOS.h>\n"
                  "\n"
                  "Studuino board;\n"
                  "\n"
                  ))
      (push! v "void setup()\n{\n")
      (for-each (^p (push! v #"  board.InitSensorPort(PORT_~p, PIDLED);\n"))
                ports)
      (push! v "  nilSysBegin();\n")
      (push! v "}\n")
      (for-each (^p (push! v p))
                '("void loop()\n"
                  "{\n"
                  "  while (1) {\n"
                  "    nilSysLock();\n"
                  "    nilSysUnlock();\n"
                  "  }\n"
                  "}\n"))
      (reverse v)))
  ;; emit C code for finalizers
  (define (emit-finalizers)
    (let1 v '()
      (push! v "NIL_THREADS_TABLE_BEGIN()\n")
      (for-each
       (^n (push! v (format "\
                        NIL_THREADS_TABLE_ENTRY(\"thread~a\", Thread~a, \
                        NULL, waThread~a, sizeof(waThread~a))\n" n n n n)))
       (iota thread-counter 1))
      (push! v "NIL_THREADS_TABLE_END()\n")
      (reverse v)))

  (define (emit-block block)
    (define (emit)
      (let1 name (block-name block)
        (case name
          ((when-green-flag-clicked)
           (when-green-flag-clicked block))
          ((forever)
           (forever block))
          ((turn-led)
           (turn-led block))
          ((wait)
           (wait block))
          (else
           (display block)
           (newline)
           ""))))
    (let1 code (emit)
      (if (list? code) code (list code))))
  (define (emit-script script)
    (emit-block script))

  (match (car json)
    (('scripts . #(scripts ...))
     (let1 code (map emit-script scripts)
       (for-each display (emit-initializers))
       (for-each (^c (for-each display c)) code)
       (for-each display (emit-finalizers))))
    (else
     (error "not a scratch scripts" json))))

;; Any op followed by port
(define (port-op? op)
  (memq op '(turn-led
             buzzer-on
             buzzer-off
             set-servomotor-degree
             set-dcmotor-power
             turn-dcmotor-on
             turn-dcmotor-off)))

(define (op->part op)
  (cond ((assq op '((turn-led . led)
                    (buzzer-on . buzzer)
                    (buzzer-off . buzzer)
                    (set-servomotor-degree . servo-motor)
                    (set-dcmotor-power . dc-motor)
                    (turn-dcmotor-on . dc-motor)
                    (turn-dcmotor-off . dc-motor)
                    (light-sensor-value . light-sensor)
                    (button-value . push-button)
                    (3-axis-digital-accelerometer
                     . 3-axis-digital-accelerometer-value)
                    (ir-photo-reflector-value . ir-photo-reflector)
                    (sound-sensor-value . sound-sensor)
                    (touch-sensor-value . touch-sensor))) => cdr)
        (else (error "uknown op code" op))))

(define (port-settings script)
  (let1 settings '()
    (define (use-port op port)
      (let1 part (op->part op)
        (cond ((assq port settings) =>
               (^s (unless (eq? part (cdr s))
                     (error "port is already used" s part))))
              (else
               (push! settings (cons port part))))))
    (define (traverse item)
      (cond ((pair? item)
             (match item
               [((? sensor-op? op) port)
                (use-port op port)]
               [((? port-op? op) port rest ...)
                (use-port op port)]
               [else #f])
             (traverse (car item))
             (traverse (cdr item)))
            (else)))
    (traverse script)
    (sort (map (^x (cons (car x) (symbol->string (cdr x)))) settings)
          (^[x y] (string<? (symbol->string x) (symbol->string y))) car)))

(define (block-list->json scripts port-mappings)
  (let1 mapped-scripts (map-ports port-mappings scripts)
    (let1 json (acons 'port-settings (port-settings mapped-scripts)
                      (acons 'scripts (blocks->json mapped-scripts) '()))
      (construct-json json))))
