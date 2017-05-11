(use-modules ((oficina common))
             ((oficina entity))
             ((oficina render)))
;; Constants
(define *gravity*   0.8165)
(define *accel*     0.166)
(define *decel*     0.3)
(define *jmpStg*    -13.0)
(define *minJmp*    -6.5)
(define *maxSpd*    0.0)
(define *defMaxSpd* 4.0)
(define *runMaxSpd* 8.5)
(define *hitboxRad* #(16.0 32.0))

;; Variables
(define *direction* 1.0)
(define *ground* #f)
(define *speed* #(0.0 0.0 0.0))

;; Components
(define *animator* #f)
(define *mastersensor* #f)
(define *bottomsensor* #f)
(define *bottomLsensor* #f)
(define *bottomRsensor* #f)
(define *ledgeLsensor* #f)
(define *ledgeRsensor* #f)
(define *leftsensor* #f)
(define *rightsensor* #f)
(define *topsensor* #f)

(define (clamp val minval maxval)
  (min (max val minval) maxval))


(define (init)
  (propset! 0 #t)
  (propset! 1 #f)
  (propset! 2 #f)
  (trl! '(128 128 0) #t)
  (set! *animator* (get-component "animator"))

  ;; Get sensors
  (set! *mastersensor*  (get-component "MasterSensor"))
  (set! *bottomsensor*  (get-component "BottomSensor"))
  (set! *bottomLsensor* (get-component "BottomLSensor"))
  (set! *bottomRsensor* (get-component "BottomRSensor"))
  (set! *ledgeLsensor*  (get-component "LedgeLSensor"))
  (set! *ledgeRsensor*  (get-component "LedgeRSensor"))
  (set! *leftsensor*    (get-component "LeftSensor"))
  (set! *rightsensor*   (get-component "RightSensor"))
  (set! *topsensor*     (get-component "TopSensor")))

(define (update dt)
  (let ((pos (pos?))
        (lstick (lstick?)))
    ;; Y axis movement
    (if (not *ground*)
        (vector-set! *speed* :y
                     (+ (vector-ref *speed* :y)
                        *gravity*)))

    (if (and *ground*
             (btntap? :pad-a))
        (begin
          (set! *ground* #f)
          (vector-set! *speed* :y *jmpStg*)))

    ;; X axis movement
    (vector-set! *speed* :x
                 (clamp
                  (+ (vector-ref *speed* :x)
                     (* (vector-ref lstick :x)
                        *accel*))
                  (- *maxSpd*) *maxSpd*))

    (if *ground*
        (cond ((= (vector-ref lstick :x) 0.0)
               (begin
                 (cond ((> (vector-ref *speed* :x) 0.0)
                        (vector-set! *speed* :x
                                     (- (vector-ref *speed* :x)
                                        *decel*)))
                       ((< (vector-ref *speed* :x) 0.0)
                        (vector-set! *speed* :x
                                     (+ (vector-ref *speed* :x)
                                        *decel*))))
                 (if (< (abs (vector-ref *speed* :x)) *decel*)
                     (vector-set! *speed* :x 0.0))))
              ((and (< (vector-ref lstick :x) 0.0)
                    (> (vector-ref *speed* :x) 0.0))
               (vector-set! *speed* :x
                            (- (vector-ref *speed* :x)
                               (* *decel* 2.0))))
              ((and (> (vector-ref lstick :x) 0.0)
                    (< (vector-ref *speed* :x) 0.0))
               (vector-set! *speed* :x
                            (+ (vector-ref *speed* :x)
                               (* *decel* 2.0))))))
    (if (btnpress? :pad-x)
        (set! *maxSpd* *runMaxSpd*)
        (set! *maxSpd* *defMaxSpd*))

    ;; Collision detection
    (set! *ground* #f)
    (let ((all-near (get-nearest)))
      (do ((i 0 (1+ i)))
          ((>= i (vector-length all-near)))
        (if (not (propget? 0 (vector-ref all-near i)))
            (let* ((obj      (vector-ref all-near i))
                   (obj-bv   (get-component "AABB" obj))
                   (solidpos (pos? obj))
                   (solidsz  (list->vector (list (mag? :x obj)
                                                 (mag? :y obj)
                                                 (mag? :z obj)))))
              ;; Ground collision
              (if (and (not *ground*)
                       (>= (vector-ref *speed* :y) 0.0)
                       (propget? 1 obj)
                       (or (overlapping? *bottomsensor* obj-bv)
                           (overlapping? *bottomLsensor* obj-bv)
                           (overlapping? *bottomRsensor* obj-bv)))
                  (begin
                    (vector-set! pos :y (- (vector-ref solidpos :y)
                                           (vector-ref *hitboxRad* :y)))
                    (vector-set! *speed* :y 0.0)
                    (set! *ground* #t)))

              ;; Top collision
              (if (and (< (vector-ref *speed* :y) 0.0)
                       (propget? 1 obj)
                       (not (propget? 2 obj))
                       (overlapping? *topsensor* obj-bv))
                  (begin
                    (vector-set! pos :y (+ (vector-ref solidpos :y)
                                           (vector-ref solidsz :y)
                                           (vector-ref *hitboxRad* :y)))
                    (vector-set! *speed* :y 0.0)))

              ;; Left collision
              (if (and (< (vector-ref *speed* :x) 0.0)
                       (propget? 1 obj)
                       (not (propget? 2 obj))
                       (overlapping? *leftsensor* obj-bv))
                  (begin
                    (vector-set! pos :x (+ (vector-ref solidpos :x)
                                           (vector-ref solidsz :x)
                                           (vector-ref *hitboxRad* :x)))
                    (vector-set! *speed* :x 0.0)))

              ;; Right collision
              (if (and (> (vector-ref *speed* :x) 0.0)
                       (propget? 1 obj)
                       (not (propget? 2 obj))
                       (overlapping? *rightsensor* obj-bv))
                  (begin
                    (vector-set! pos :x (- (vector-ref solidpos :x)
                                           (vector-ref *hitboxRad* :x)))
                    (vector-set! *speed* :x 0.0)))))))

    ;; Platformer-like jump
    (if (and (not *ground*)
             (< (vector-ref *speed* :y) *minJmp*)
             (not (btnpress? :pad-a)))
        (vector-set! *speed* :y *minJmp*))

    ;; Transform position
    (vector-set! pos :x (+ (vector-ref pos :x) (vector-ref *speed* :x)))
    (vector-set! pos :y (+ (vector-ref pos :y) (vector-ref *speed* :y)))
    (vector-set! pos :z (+ (vector-ref pos :z) (vector-ref *speed* :z)))

    ;; Hand position back to engine
    (trl! (vector->list pos) #t)

    ;; Direction
    (cond ((> (vector-ref *speed* :x) 0.0)
           (set! *direction* 1.0))
          ((< (vector-ref *speed* :x) 0.0)
           (set! *direction* -1.0)))
    (scl! (list *direction* 1.0 1.0) #t)

    ;; Animation
    (if *ground*
        (begin
          (if (= (vector-ref *speed* :x) 0.0)
              (set-anim! *animator* "stopped")
              (begin
                (set-anim! *animator* "walking")
                (let* ((norm (- 1.0 (/ (abs (vector-ref *speed* :x))
                                       *runMaxSpd*)))
                       (animspd (* norm 6.0)))
                  (set-anim-spd! *animator* (+ animspd
                                               (anim-def-spd? *animator*)))))))
        (set-anim! *animator* "jumping"))))
                         
                    
    
        
