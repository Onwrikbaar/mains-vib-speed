# mains-vib-speed
Electronic circuit and Arduino sketch for speed control of mains-powered motors - like the classic Hitachi wand.

WARNING<br/>
The most important thing during construction, testing and usage of this design is to always maintain sufficient electrical separation and insulation between the mains voltage side and the microcontroller side. Since this circuit works with potentially deadly voltages, all applicable precautions must be taken.

EXPECTATION MANAGEMENT</br>
This is just a demonstrator. The electronics can be improved and the software is absolutely barebones.</br>
See it in action [on YouTube](https://www.youtube.com/watch?v=LFKG1e9KCeg).

Remarks (refer to the schematic):
* Pins 1 and 2 of the SFH620 optoisolator are equivalent; they may be swapped if that is convenient for PCB layout.
* The 316 Ω resistor's value is not critical, 330 Ω is fine too. But do observe its voltage rating.
* The two 18 kΩ resistors get hot, so leave some room around them. Maybe elevate them a little from the board.
* The fuse should probably be a bit smaller than 1A, since the photo-triac is rated for 0.9A. I only had 1A types lying around. I blew one and the triac survived.

Farnell order codes:
- 1870800 -> VO2223-X001 photo-triac
- 1469531 -> SFH620A AC-input optoisolator
- 2500238 -> 10 nF, 630V capacitor
- 9467246 -> 316 Ω, 350V, 0.6W resistor
- 1738655 -> 18 kΩ, 350V, 2W resistor (2x)

Sorry that the circuit diagram is hand drawn. Will fix that once I have a little more time.
