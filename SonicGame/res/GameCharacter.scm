;;;; GameCharacter.scm
;;;; Sonic behaviour

(define *paused* #f)
(define *super* #f)

(define *speed* (make-vector 2 0.0))
(define *top-spd* (make-vector 2 0.0))
(define *max-spd* (make-vector 2 0.0))
(define *accel* 0.046875)
(define *air-accel* 0.1875)
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


(define def-player-values
  (lambda ()
    (if (not *super*)
	;; Sonic
	(begin
	  (set! *accel* 0.046875)
	  (set! *air-accel* 0.1875)
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
    (animation-setrunning! (not *paused*))
    (set! *super* #f)
    (setsuper! *super*)
    (def-player-values)
    (trl! '((/ 640.0 16.0)
	    (/ 360.0 2.0)
	    0.0)
	  #t +this+)))

(define update
  (lambda (dt)
    ;; Pause handle
    (if (btntap? :pad-start :player-one)
	(begin
	  (set! *paused* (not *paused*))
	  (animation-setrunning! (not *paused*))))
    
    (if (not *paused*)
	(begin	  
	  (let ((lstk (lstick? :player-one)))
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
			      (animation-set! "Skid"))
			  (set! *action* :act-skid)
			  (set! currspd (- currspd *decel*)))
			)
		    (if (and (< currspd 0.0)
			     (> (vector-ref lstk :x) 0.0))
			(begin
			  (if (> (abs currspd) 1.8)
			      (animation-set! "Skid"))
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
	      (if (and (<= (- (vector-ref (pos? +this+) :x) 10.0) 0.0)
		       (< (vector-ref lstk :x) 0.0))
		  (begin
		    (set! currspd 0.0)
		    (trl! '(10.0 (vector-ref (pos? +this+) :y) 0.0) #t +this+))
		  
		  )
	      
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
		     ))
		  ;; Other animation speed settings
		  (begin
		    (cond
		     ((= *action* :act-jmp)
		      (let ((spd-perc (/ currspd 12.0)))
			(animation-setspd!
			 (- (animation-defspd?)
			    (* (- (animation-defspd?) 1.0)
			       spd-perc)))
			)
		      )
		    )
		  )))

	    
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

	    )

	  ;; Super state
	  (if (btntap? :pad-y :player-one)
	      (begin
		(set! *super* (not *super*))
		(setsuper! *super*)
		(def-player-values)))
	  
	  ))))
