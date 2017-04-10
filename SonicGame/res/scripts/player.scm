;;;; player.scm
;;;; Sonic behaviour

(define-module (ofsonic player)
  :export (init update))
(use-modules ((oficina common))
             ((oficina entity))
             ((oficina render)))

(define *paused* #f)
(define *super* #f)

(define *current-player* :player-one)

(define *animator* #f)

(define *speed* (make-vector 2 0.0))
(define *top-spd* (make-vector 2 0.0))
(define *max-spd* (make-vector 2 0.0))
(define *accel* 0.046875)
(define *air-accel* 0.1875)
(define *airdrag* 0.96875)
(define *airdrag-minx* 0.0125)
(define *airdrag-miny* -4.0)
(define *decel* 0.5)
(define *grav*  0.21875)
(define *jmpstr* -6.5)
(define *minjmp* -4.0)

(define *ground* #t)
(define *direction* 1.0)

(define :act-none 0)
(define :act-jmp  1)
(define :act-skid 2)
(define :act-lookup 3)
(define :act-crouch 4)
(define :act-push 5)
(define *action* :act-none)

(define *rightbound* 3840.0)
(define *fakeground-y* 192.0)

(define clamp
  (lambda (val min max)
    (if (< val min) min (if (> val max) max val))))


(define def-player-values
  (lambda ()
    (if (not *super*)
        ;; Sonic
        (begin
          (set! *accel* 0.046875)
          (set! *air-accel* 0.1875)
          (set! *airdrag* 0.96875)
          (set! *airdrag-minx* 0.0125)
          (set! *airdrag-miny* -4.0)
          (vector-set! *max-spd* :x 12.0)
          (vector-set! *top-spd* :x 6.0)
          (set! *decel* 0.5)
          (set! *grav* 0.21875)
          (set! *jmpstr* -6.5)
          (set! *minjmp* -4.0))
        ;; Super Sonic
        (begin
          (set! *accel* 0.1875)
          (set! *air-accel* 0.375)
          (vector-set! *top-spd* :x 10.0)
          (set! *decel* 1.0)
          (set! *jmpstr* -8.0)))
    ))





(define init
  (lambda () 
    (set! *action* :act-none)
    ;;(set-super! #f)
    (set! *animator* (get-component "Animator"))
    (def-player-values)
    (set! *direction* 1.0)
    (trl! (list 128.0 *fakeground-y* 0.0) #t)))

(define update
  (lambda (dt)
    ;; Pause handle
    (if (btntap? :pad-start *current-player*)
        (begin
          (set! *paused* (not *paused*))
          (set-anim-running! *animator* (not *paused*))))
    
    (if (not *paused*)
        (begin    
          (let ((lstk (lstick? *current-player*)))
            ;; Acceleration
            (if (and (< (abs (vector-ref *speed* :x))
                        (vector-ref *top-spd* :x))
                     (or (= *action* :act-none)
                         (= *action* :act-jmp)
                         (= *action* :act-skid)))
                (vector-set! *speed* :x
                             (+ (vector-ref *speed* :x)
                                (* (vector-ref lstk :x)
                                   (if *ground*
                                       *accel*
                                       *air-accel*)))))

            ;; X Axis
            (let ((currspd (vector-ref *speed* :x)))
              ;; Default deceleration
              (if (and *ground*
                       (= (vector-ref lstk :x) 0.0))     
                  (cond
                   ((and (> currspd (- *accel*))
                         (< currspd *accel*))
                    (set! currspd 0.0))
                   ((< currspd 0.0)
                    (set! currspd (+ currspd *accel*)))
                   ((> currspd 0.0)
                    (set! currspd (+ currspd (- *accel*))))))
              ;; Skidding
              (if *ground*
                  (begin
                    (if (and (> currspd 0.0)
                             (< (vector-ref lstk :x) 0.0))
                        (begin
                          (if (> (abs currspd) 1.8)
                              (set-anim! *animator* "Skid"))
                          (set! *action* :act-skid)
                          (set! currspd (- currspd *decel*)))
                        )
                    (if (and (< currspd 0.0)
                             (> (vector-ref lstk :x) 0.0))
                        (begin
                          (if (> (abs currspd) 1.8)
                              (set-anim! *animator* "Skid"))
                          (set! *action* :act-skid)
                          (set! currspd (+ currspd *decel*)))
                        )
                    
                    ;; Reset skidding
                    (if (and (= *action* :act-skid)
                             (> currspd (- *decel*))
                             (< currspd *decel*))
                        (begin
                          (set! *action* :act-none)
                          (set! currspd 0.0)))
                    ))
              ;; Left boundary limit
              (if (and (<= (- (vector-ref (pos?) :x) 10.0) 0.0)
                       (< currspd 0.0))
                  (begin
                    (set! currspd 0.0)
                    (trl! (list 10.0 (vector-ref (pos?) :y) 0.0) #t)))

              ;; Right boundary limit (temporary)
              (if (and (>= (+ (vector-ref (pos?) :x) 10.0) *rightbound*)
                       (> currspd 0.0))
                  (begin
                    (set! currspd 0.0)
                    (trl! (list (- *rightbound* 10.0)
                                (vector-ref (pos?) :y)
                                0.0) #t)))
              
              ;; Give back X speed
              (set! currspd (clamp currspd
                                   (- (vector-ref *max-spd* :x))
                                   (vector-ref *max-spd* :x)))
              (vector-set! *speed* :x currspd))



            
            ;; Y Axis
            (let ((currspd (vector-ref *speed* :y)))
              (if (not *ground*)
                  (begin
                    ;; Gravity Action
                    (set! currspd (+ currspd *grav*))
                    
                    ;; Fake ground
                    (let ((mypos (pos?)))
                      (if (>= (vector-ref mypos :y)
                              *fakeground-y*)
                          (begin
                            (set! *ground* #t)
                            (set! currspd 0.0)
                            (set! *action* :act-none)
                            )))
                    
                    ;; Minimum jump strength
                    (if (and (= *action* :act-jmp)
                             (not (btnpress? :pad-a *current-player*))
                             (< currspd *minjmp*))
                        (set! currspd *minjmp*))
                    )
                  ;; Else
                  (begin
                    ;; Jumping
                    (if (and (or (= *action* :act-none)
                                 (= *action* :act-lookup)
                                 (= *action* :act-push))
                             (btntap? :pad-a *current-player*))
                        (begin
                          (set-anim! *animator* "Roll")
                          (set! *ground* #f)
                          (set! currspd *jmpstr*)
                          (set! *action* :act-jmp)))
                    (let ((ypos
                           (- *fakeground-y*
                              (* 2 (cos (/ (- (vector-ref (pos?) :x)
                                              (* (truncate
                                                  (/ (vector-ref (pos?) :x) 128))
                                                 128)) 24.0))))))
                      (trl! (list (vector-ref (pos?) :x)
                                  ypos
                                  0.0)
                            #t))
                    ))
              ;; Give back Y speed
              (vector-set! *speed* :y currspd))


            ;; Air Drag
            (if (and (< (vector-ref *speed* :y) 0.0)
                     (> (vector-ref *speed* :y) *airdrag-miny*)
                     (>= (abs (vector-ref *speed* :x)) *airdrag-minx*))
                (vector-set! *speed* :x
                             (* (vector-ref *speed* :x) *airdrag*)))
            
            
            
            ;; Add speed to position
            (trl! (list (vector-ref *speed* :x)
                        (vector-ref *speed* :y)
                        0.0) #f)

            ;; Check for super state
            (if (not (eq? *super* (super?)))
                (begin
                  (set! *super* (super?))
                  (def-player-values)))
            
            
            ;; Change animation according to speed
            (let ((currspd (abs (vector-ref *speed* :x))))
              (if (and *ground*
                       (or (= *action* :act-none)
                           (= *action* :act-crouch)
                           (= *action* :act-lookup)
                           (= *action* :act-push)))
                  (begin
                    (cond
                     ((= currspd 0.0)
                      (cond
                       ((= (vector-ref lstk :y) 0.0)
                        (cond
                         ((and (> (vector-ref lstk :x) 0.0)
                               (>= (+ (vector-ref (pos?) :x) 10.0)
                                   *rightbound*))
                          (begin
                            (set-anim! *animator* "Push")
                            (set! *action* :act-push)))
                         (else
                          (begin
                            (set-anim! *animator* "Idle")
                            (set! *action* :act-none)))))
                       ((> (vector-ref lstk :y) 0.0)
                        (begin
                          (set-anim! *animator* "Crouch")
                          (set! *action* :act-crouch)))
                       ((< (vector-ref lstk :y) 0.0)
                        (begin
                          (set-anim! *animator* "LookUp")
                          (set! *action* :act-lookup)))))

                     ((>= currspd 9.95)
                      (set-anim! *animator* "Peel"))
                     
                     ((>= currspd 5.9)
                      (set-anim! *animator* "Run"))
                     
                     (else
                      (begin
                        (let ((spd-perc (/ currspd 5.9)))
                          (set-anim! *animator* "Walk")
                          (set-anim-spd! *animator*
                                         (- (anim-def-spd? *animator*)
                                            (* (- (anim-def-spd? *animator*) 3.0)
                                               spd-perc)))
                          )))
                     ))
                  ;; Other animation speed settings
                  (begin
                    (cond
                     ((= *action* :act-jmp)
                      (let ((spd-perc (/ currspd 12.0)))
                        (set-anim-spd! *animator*
                                       (- (anim-def-spd? *animator*)
                                          (* (- (anim-def-spd? *animator*) 1.0)
                                             spd-perc)))
                        )
                      )
                     )
                    )))

            
            ;; Change direction according to speed
            (if (or (= *action* :act-none)
                    (= *action* :act-jmp))
                (cond
                 ((> (vector-ref lstk :x) 0.0)
                  (set! *direction* 1.0))
                 ((< (vector-ref lstk :x) 0.0)
                  (set! *direction* -1.0))))
            (scl! (list *direction* 1.0 1.0) #t)
            )

          ;; Super state
          (if (btntap? :pad-y *current-player*)
              (begin
                (set-super! (not (super?)))
                (def-player-values)))
          
          ))))
