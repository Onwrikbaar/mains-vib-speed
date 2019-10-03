# mains-vib-speed
Electronic circuit and Arduino sketch for speed control of mains-powered motors - like the classic Hitachi wand.

Remarks:
* Pins 1 and 2 of the SFH620 optoisolator are equivalent, they may be reversed if that is convenient for PCB layout.
* The 316 Ω resistor's value is not critical, 330 Ω is fine too. But do observe its voltage rating.
* The two 18 kΩ resistors get hot.Leave some room around them. Maybe elevate them a little from the board.
* The fuse should probably be a bit smaller than 1A, since the photo-triac is rated for 0.9A. I only had 1A types lying around. I blew one and the triac survived.

Farnell order codes:
- 1870800 -> VO2223-X001 photo-triac
- 1469531 -> SFH620A AC-input optoisolator
- 2500238 -> 10 nF, 630V capacitor
- 9467246 -> 316 Ω, 350V, 0.6W resistor
- 1738655 -> 18 kΩ, 350V, 2W resistor (2x)

The most important thing during construction is of course the separation between the mains voltage side and the microcontroller side.
