-----------------------------------------------------------------
Simple navtex decoder plugin
-----------------------------------------------------------------

![overview](/navtex-example.png?raw=true)

-----------------------------------------------------------------------
Navtex
-----------------------------------------------------------------------

NAVTEX (NAVigational TEleX) is a service for delivery of navigational and meteorological warnings and forecasts, as well as urgent maritime safety information (MSI) to ships. 

The transmissions are layered on top of SITOR collective B-mode. SITOR-B is a forward error correcting (FEC) broadcast that uses the CCIR 476 character set. NAVTEX messages are transmitted at 100 baud using FSK modulation with a frequency shift of 170 Hz.

The NavTex plugin can be used to receive and decode these messages,
the plugin will set tuning to 518KHz, the standard frequency for 
navtex messages.

Another frequency for navtex messages is 490 KHz.

The plugin shows - next to a label for the received text - a few
number displays, some comboboxes and a dump button.

The number displays give - from left to right - an indication of the
signalstrength and the frequency correction applied to the signal (assuming
the afc is on).

The combo boxes, from  left to right

 * selector for the afc on or off;

 * selector for reversing the mark and sppace frequencies in the signal;

 * selector for switching on or off the Fec (Forward error correction)

 * selector for switching on all data or the navtex messages

Due to the way the plugin is implemented, even if there are no navtex
messages, some text is shown, usually garbage.
The 3-d selector switches the Forward error correction, the 4-th selector
determines whether or not all text is shown, or only the text of validly
decoded navtex messages.

 * the dump button, when touched will show a file selection menu. A file can be selected where the text received will be stored. Of course, useful in
combination with settings of the 3-d ssnd 4-th selector.
Touching the button when dumpiung will close the dump file.

The implementation of the decoder is taken from the navtex decoder
in the swradio.
