;; recurse save-character on Ascii codes from 33 to 125
(define (sfsfrec inAscii inFont inFontsize inDir)
  (if (<= inAscii 125)
      (
       (save-character inAscii inFont inFontsize inDir)
       (sfsfrec (+ inAscii 1) inFont inFontsize inDir)
       )
      ()
      )
  )

(define (script-fu-save-font inFont inFontsize inDir)
  (sfsfrec 33 inFont inFontsize inDir)
  )

;; save a character by its ascii code into the given dir
(define (save-character inAscii inFont inFontSize inDir)
  (let*
      (
					; define our local variables
					; create a new image:
       (theImageWidth  1)
       (theImageHeight 1)
       (theImage (car
		  (gimp-image-new
		   theImageWidth
		   theImageHeight
		   RGB
		   )
		  )
                 )
       (theText)
       (theFontSize inFontSize)
       (theLayer
	(car
	 (gimp-layer-new
	  theImage
	  theImageWidth
	  theImageHeight
	  RGBA-IMAGE
	  "layer 1"
	  100
	  NORMAL
	  )
	 )
	)
;       used to crop the image horizontally but not vertically
       (theBaseLayer
	(car
	 (gimp-layer-new
	  theImage
	  theImageWidth
	  theImageHeight
	  RGBA-IMAGE
	  "layer 0"
	  100
	  NORMAL
	  )
	 )
	)
       (theFilename (string-append inDir "/" (number->string inAscii) ".png"))
       )
    
					; the function itself
; create an image with the character and resize it
    (gimp-image-add-layer theImage theLayer 0)
    (gimp-image-add-layer theImage theBaseLayer 1)
    (gimp-context-set-background '(0 0 0))
    (gimp-context-set-foreground '(255 255 255))
    (gimp-drawable-fill theLayer TRANSPARENT-FILL)
    
       (set! theText
       	     (car
       	      (gimp-text-fontname
       	       theImage theLayer
       	       0 0
       	       (string (integer->char inAscii))
       	       0
       	       TRUE ; Anti-aliasing
       	       theFontSize PIXELS
       	       inFont)
       	      )
       	     )

       (set! theImageHeight  (car (gimp-drawable-height theText) ) )

       	(set! theImageWidth   (car (gimp-drawable-width  theText) ) )

        (gimp-image-resize theImage theImageWidth theImageHeight 0 0)
	
	(gimp-layer-resize theLayer theImageWidth theImageHeight 0 0)

	(gimp-layer-resize theBaseLayer theImageWidth theImageHeight 0 0)

       (gimp-floating-sel-anchor theText)


       ;; Height is resized before autocropping only to keep V-alignment of characters
	;; Width is resized after to remove whitespaces

	(plug-in-autocrop-layer RUN-NONINTERACTIVE theImage theLayer)

	;; Character has to be pushed to the left
	(let* (
	       (ox (car (gimp-drawable-offsets theLayer)))
	       (oy (cadr (gimp-drawable-offsets theLayer)))
	       )
	  
	  (gimp-layer-translate theLayer (- ox) 0)
	  )
	
        (set! theImageWidth   (car (gimp-drawable-width  theLayer) ) )

        (gimp-image-resize theImage theImageWidth theImageHeight 0 0)
	
        (gimp-layer-resize theBaseLayer theImageWidth theImageHeight 0 0)

	(gimp-image-merge-visible-layers theImage EXPAND-AS-NECESSARY)
	
;	(gimp-display-new theImage)

;       (gimp-image-clean-all theImage) ; To prevent save prompt during testing

       (file-png-save-defaults RUN-NONINTERACTIVE theImage (car (gimp-image-get-active-drawable theImage)) theFilename theFilename)
       
    )
  )


(script-fu-register
    "script-fu-save-font"                        ;func name
    "Save Font as Images"                                  ;menu label
    "Creates pngs for each character in a\
      font. Characters are white on a transparent\
      background."              ;description
    "Benjamin Wack"                             ;author
    "copyright 2010, Benjamin Wack"        ;copyright notice
    "November 25, 2010"                          ;date created
    ""                     ;image type that the script works on
;    SF-ADJUSTMENT  "Ascii code"          '(65 32 125 1 10 0 SF-SPINNER)   ;an ascii code
    SF-FONT        "Font"          "Sans"    ;a font variable
    SF-ADJUSTMENT  "Font size"     '(40 1 1000 1 10 0 1)
    SF-DIRNAME     "Directory"     "D:/project6014/fontgen"
;    SF-COLOR       "Color"         '(0 0 0)     ;color variable
  )
  (script-fu-menu-register "script-fu-save-font" "<Image>/File/Create")
