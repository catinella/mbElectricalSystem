SamacSys ECAD Model
18586926/766822/2.50/9/4/MOSFET P-Channel

DESIGNSPARK_INTERMEDIATE_ASCII

(asciiHeader
	(fileUnits MM)
)
(library Library_1
	(padStyleDef "r145_85"
		(holeDiam 0)
		(padShape (layerNumRef 1) (padShapeType Rect)  (shapeWidth 0.850) (shapeHeight 1.450))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 0) (shapeHeight 0))
	)
	(padStyleDef "r105_85"
		(holeDiam 0)
		(padShape (layerNumRef 1) (padShapeType Rect)  (shapeWidth 0.850) (shapeHeight 1.050))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 0) (shapeHeight 0))
	)
	(padStyleDef "r470_375"
		(holeDiam 0)
		(padShape (layerNumRef 1) (padShapeType Rect)  (shapeWidth 3.750) (shapeHeight 4.700))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 0) (shapeHeight 0))
	)
	(textStyleDef "Default"
		(font
			(fontType Stroke)
			(fontFace "Helvetica")
			(fontHeight 50 mils)
			(strokeWidth 5 mils)
		)
	)
	(patternDef "TPH4R003NLL1Q" (originalName "TPH4R003NLL1Q")
		(multiLayer
			(pad (padNum 1) (padStyleRef r145_85) (pt -1.905, -2.750) (rotation 0))
			(pad (padNum 2) (padStyleRef r145_85) (pt -0.635, -2.750) (rotation 0))
			(pad (padNum 3) (padStyleRef r145_85) (pt 0.635, -2.750) (rotation 0))
			(pad (padNum 4) (padStyleRef r145_85) (pt 1.905, -2.750) (rotation 0))
			(pad (padNum 5) (padStyleRef r105_85) (pt 1.905, 2.950) (rotation 0))
			(pad (padNum 6) (padStyleRef r105_85) (pt 0.635, 2.950) (rotation 0))
			(pad (padNum 7) (padStyleRef r105_85) (pt -0.635, 2.950) (rotation 0))
			(pad (padNum 8) (padStyleRef r105_85) (pt -1.905, 2.950) (rotation 0))
			(pad (padNum 9) (padStyleRef r470_375) (pt 0.000, 0.550) (rotation 90))
		)
		(layerContents (layerNumRef 18)
			(attr "RefDes" "RefDes" (pt 0.000, 0.000) (textStyleRef "Default") (isVisible True))
		)
		(layerContents (layerNumRef 28)
			(line (pt -2.5 -2.5) (pt 2.5 -2.5) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt 2.5 -2.5) (pt 2.5 2.5) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt 2.5 2.5) (pt -2.5 2.5) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt -2.5 2.5) (pt -2.5 -2.5) (width 0.1))
		)
		(layerContents (layerNumRef 30)
			(line (pt -3.5 4.475) (pt 3.5 4.475) (width 0.1))
		)
		(layerContents (layerNumRef 30)
			(line (pt 3.5 4.475) (pt 3.5 -4.475) (width 0.1))
		)
		(layerContents (layerNumRef 30)
			(line (pt 3.5 -4.475) (pt -3.5 -4.475) (width 0.1))
		)
		(layerContents (layerNumRef 30)
			(line (pt -3.5 -4.475) (pt -3.5 4.475) (width 0.1))
		)
		(layerContents (layerNumRef 18)
			(line (pt -1.905 -4) (pt -1.905 -4) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(arc (pt -1.905, -4.05) (radius 0.05) (startAngle 90.0) (sweepAngle 180.0) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt -1.905 -4.1) (pt -1.905 -4.1) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(arc (pt -1.905, -4.05) (radius 0.05) (startAngle 270) (sweepAngle 180.0) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt -1.905 -4) (pt -1.905 -4) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(arc (pt -1.905, -4.05) (radius 0.05) (startAngle 90.0) (sweepAngle 180.0) (width 0.2))
		)
	)
	(symbolDef "TPCA8120_L1Q" (originalName "TPCA8120_L1Q")

		(pin (pinNum 1) (pt 0 mils 0 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -25 mils) (rotation 0]) (justify "Left") (textStyleRef "Default"))
		))
		(pin (pinNum 2) (pt 0 mils -100 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -125 mils) (rotation 0]) (justify "Left") (textStyleRef "Default"))
		))
		(pin (pinNum 3) (pt 0 mils -200 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -225 mils) (rotation 0]) (justify "Left") (textStyleRef "Default"))
		))
		(pin (pinNum 4) (pt 0 mils -300 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -325 mils) (rotation 0]) (justify "Left") (textStyleRef "Default"))
		))
		(pin (pinNum 5) (pt 0 mils -400 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -425 mils) (rotation 0]) (justify "Left") (textStyleRef "Default"))
		))
		(pin (pinNum 6) (pt 1000 mils 0 mils) (rotation 180) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 770 mils -25 mils) (rotation 0]) (justify "Right") (textStyleRef "Default"))
		))
		(pin (pinNum 7) (pt 1000 mils -100 mils) (rotation 180) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 770 mils -125 mils) (rotation 0]) (justify "Right") (textStyleRef "Default"))
		))
		(pin (pinNum 8) (pt 1000 mils -200 mils) (rotation 180) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 770 mils -225 mils) (rotation 0]) (justify "Right") (textStyleRef "Default"))
		))
		(pin (pinNum 9) (pt 1000 mils -300 mils) (rotation 180) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 770 mils -325 mils) (rotation 0]) (justify "Right") (textStyleRef "Default"))
		))
		(line (pt 200 mils 100 mils) (pt 800 mils 100 mils) (width 6 mils))
		(line (pt 800 mils 100 mils) (pt 800 mils -500 mils) (width 6 mils))
		(line (pt 800 mils -500 mils) (pt 200 mils -500 mils) (width 6 mils))
		(line (pt 200 mils -500 mils) (pt 200 mils 100 mils) (width 6 mils))
		(attr "RefDes" "RefDes" (pt 850 mils 300 mils) (justify Left) (isVisible True) (textStyleRef "Default"))

	)
	(compDef "TPCA8120,L1Q" (originalName "TPCA8120,L1Q") (compHeader (numPins 9) (numParts 1) (refDesPrefix Q)
		)
		(compPin "1" (pinName "S_1") (partNum 1) (symPinNum 1) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "2" (pinName "S_2") (partNum 1) (symPinNum 2) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "3" (pinName "S_3") (partNum 1) (symPinNum 3) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "4" (pinName "G") (partNum 1) (symPinNum 4) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "5" (pinName "D_1") (partNum 1) (symPinNum 5) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "6" (pinName "D_2") (partNum 1) (symPinNum 6) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "7" (pinName "D_3") (partNum 1) (symPinNum 7) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "8" (pinName "D_4") (partNum 1) (symPinNum 8) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "9" (pinName "D_5") (partNum 1) (symPinNum 9) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(attachedSymbol (partNum 1) (altType Normal) (symbolName "TPCA8120_L1Q"))
		(attachedPattern (patternNum 1) (patternName "TPH4R003NLL1Q")
			(numPads 9)
			(padPinMap
				(padNum 1) (compPinRef "1")
				(padNum 2) (compPinRef "2")
				(padNum 3) (compPinRef "3")
				(padNum 4) (compPinRef "4")
				(padNum 5) (compPinRef "5")
				(padNum 6) (compPinRef "6")
				(padNum 7) (compPinRef "7")
				(padNum 8) (compPinRef "8")
				(padNum 9) (compPinRef "9")
			)
		)
		(attr "Manufacturer_Name" "Toshiba")
		(attr "Manufacturer_Part_Number" "TPCA8120,L1Q")
		(attr "Mouser Part Number" "757-TPCA8120L1Q")
		(attr "Mouser Price/Stock" "https://www.mouser.co.uk/ProductDetail/Toshiba/TPCA8120L1Q?qs=B6kkDfuK7%2FD2wO%2FgucdpPg%3D%3D")
		(attr "Arrow Part Number" "")
		(attr "Arrow Price/Stock" "")
		(attr "Description" "MOSFETs Silicon P-Channel MOS (U-MOS)")
		(attr "Datasheet Link" "https://www.mouser.com/datasheet/2/408/TPCA8120_datasheet_en_20140214-3223135.pdf")
		(attr "Height" "1 mm")
	)

)
