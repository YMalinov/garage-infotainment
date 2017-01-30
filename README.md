garage-infotainment
==========

A spinoff project based on <stong>car-infotainment</strong> with no compass and small misc changes.

The following modules are used:
<ul>
	<li>1x Arduino UNO r3 board </li>
	<li>2x DS18B20 water proof temperature sensors </li>
	<li>1x AltIMU board </li>
	<li>1x 16x2 SerLCD display </li>
</ul>
<br/>
Currently the following libraries are used:
<ul>
	<li>LPS pressure sensor library from Pololu</li>
	<li>OneWire (for the DS18B20 temp sensors)</li>
	<li>Dallas Temperature Control (for the DS18S20 temp sensors)</li>
</ul>
<br/>
If setup correctly, the screen should display the information in the following way (without the quotes):
<ul>
	<li><pre>"I: 25.0 O: -5.0*"</pre> (input and output temperature, respectively (the star at the end will be shown, if outside temperatures are under 4.0 degrees) (also, the display will output "risk of ice" for a duration of approx. 8 seconds upon startup, if the temperatures are below the freezing point))</li>
	<li><pre>"500 kPa        ^"</pre> (atmospheric pressure in kilopascals; and display refresh animation)</li>
</ul>
<br/>
