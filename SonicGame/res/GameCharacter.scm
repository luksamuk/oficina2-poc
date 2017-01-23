;;;; GameCharacter.scm
;;;; Sonic behaviour

(define *speed* (make-vector 2 0.0))
(define *top-spd* (make-vector 2 0.0))
(define *max-spd* (make-vector 2 0.0))
(define *accel* 0.046875)
(define *decel* 0.5)
(define *grav*  0.21875)
(define *jmpstr* -6.5)
(define *minjmp* -4.0)

(define *ground* #t)

(define :act-none 0)
(define :act-jmp  1)
(define :act-skid 2)
(define :act-lookup 3)
(define :act-crouch 4)
(define *action* :act-none)


(define clamp
  (lambda (val min max)
    (if (< val min) min (if (> val max) max val))))



(define init
  (lambda ()
    (set! *action* :act-none) 
    (trl! '((/ 640.0 16.0)
	    (/ 360.0 2.0)
	    0.0)
	  #t +this+)))

(define update
  (lambda (dt)
    (let ((lstk (lstick? :player-one)))
      ;; Acceleration
      (if (or (= *action* :act-none)
	      (= *action* :act-jmp))
	  (vector-set! *speed* :x
		       (+ (vector-ref *speed* :x)
			  (* (vector-ref lstk :x) *accel*))))

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
		    (animation-set! "Skid")
		    (set! *action* :act-skid)
		    (set! currspd (- currspd *decel*)))
		  )
	      (if (and (< currspd 0.0)
		       (> (vector-ref lstk :x) 0.0))
		  (begin
		    (animation-set! "Skid")
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
	(if (and (<= (vector-ref (pos? +this+) :x) 0.0)
		 (< (vector-ref lstk :x) 0.0))
	    (set! currspd 0.0))
	;; Give back X speed
	(set! currspd (clamp currspd -12.0 12.0))
	(vector-set! *speed* :x currspd))



      
      ;; Y Axis
      (let ((currspd (vector-ref *speed* :y)))
	(if (not *ground*)
	    (begin
	      ;; Gravity Action
	      (set! currspd (+ currspd *grav*))
	      
	      ;; Fake ground
	      (let ((mypos (pos? +this+)))
		(if (>= (vector-ref mypos :y)
			(/ 360.0 2.0))
		    (begin
		      (set! *ground* #t)
		      (set! currspd 0.0)
		      (trl! '((vector-ref mypos :x)
			      (/ 360.0 2.0)
			      0.0)
			    #t +this+)
		      (set! *action* :act-none)
		      )))
	      
	      ;; Minimum jump strength
	      (if (and (= *action* :act-jmp)
		       (not (btnpress? :pad-a :player-one))
		       (< currspd *minjmp*))
		  (set! currspd *minjmp*))
	      )
	    ;; Else
	    (begin
	      ;; Jumping
	      (if (and (or (= *action* :act-none)
			   (= *action* :act-lookup))
		       (btntap? :pad-a :player-one))
		  (begin
		    (animation-set! "Roll")
		    (set! *ground* #f)
		    (set! currspd *jmpstr*)
		    (set! *action* :act-jmp)))
	      ))
	;; Give back Y speed
	(vector-set! *speed* :y currspd))


      
      
      ;; Add speed to position
      (trl! '((vector-ref *speed* :x)
	      (vector-ref *speed* :y)
	      0.0) #f +this+)

      ;; Change animation according to speed
      (let ((currspd (abs (vector-ref *speed* :x))))
	(if (and *ground*
		 (or (= *action* :act-none)
		     (= *action* :act-crouch)
		     (= *action* :act-lookup)))
	    (begin
	      (cond
	       ((= currspd 0.0)
		(cond
		 ((= (vector-ref lstk :y) 0.0)
		  (begin
		    (animation-set! "Idle")
		    (set! *action* :act-none)))
		 ((> (vector-ref lstk :y) 0.0)
		  (begin
		    (animation-set! "Crouch")
		    (set! *action* :act-crouch)))
		 ((< (vector-ref lstk :y) 0.0)
		  (begin
		    (animation-set! "LookUp")
		    (set! *action* :act-lookup)))))

	       ((>= currspd 9.95)
		(animation-set! "Peel"))
	       
	       ((>= currspd 5.9)
		(animation-set! "Run"))
	       
	       (else
		(begin
		  (let ((spd-perc (/ currspd 5.9)))
		    (animation-set! "Walk")
		    (animation-setspd!
		     (- (animation-defspd?)
			(* (- (animation-defspd?) 3.0)
			   spd-perc)))
		    )))
	       )))
	)

      
      ;; Change direction according to speed
      (let ((currspd (vector-ref *speed* :x)))
	(cond
	 ((> currspd 0.0)
	  (begin
	    (scl! '(1.0 1.0 1.0) #t +this+)))
	 ((< currspd 0.0)
	  (begin	
	    (scl! '(-1.0 1.0 1.0) #t +this+))))
	)

      )))
