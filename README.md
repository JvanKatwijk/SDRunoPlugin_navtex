-----------------------------------------------------------------
Simple navtex decoder plugin
-----------------------------------------------------------------

The navtex plugin for SDRuno is - as the name suggests - a plugin for
decoding navtext signals transmitted on shortwave.

![overview](/navtex-plugin-widget.png?raw=true)

-----------------------------------------------------------------------------
  READ THIS FIRST installing the plugin
-----------------------------------------------------------------------------

Since the navtex signals are small band signal (100 Hz in this plugin),
the samplerate used as input for the plugin is *192000* samples/second.

The 192000 samplerate is provided by the SDRuno platform, further
decimation and filtering is dome by the plugin itself.
Note that on a 2 MHz wide spectrum, it is pretty hard to detect small
signals, like the navtex one, so use the zooming facility on the
main spectrum window to show a smaller spectrum.

The plugin itself can be stored in the folder for community plugins

The plugin is - as other plugins - developed under MSVC. Its functioning
depends on lots of other "dll's" (Dynamic Load Libraries);

If a 0x000012f error is encountered on trying to load the plugin,
it means that dll's from the Visual C++ Redistributable(VS 2015) are
not found.

Installing seems to be simple:

How do I fix the api-ms-win-crt-runtime-l1-1-0. dll missing error?

    Install the software via Windows Update.
    Download Visual C++ Redistributable for Visual Studio 2015 from Microsoft directly.
    Install or Repair the Visual C++ Redistributable for Visual Studio 2015 on your computer.

-----------------------------------------------------------------------
Navtex
-----------------------------------------------------------------------

NAVTEX (NAVigational TEleX) is a service for delivery of navigational and meteorological warnings and forecasts, as well as urgent maritime safety information (MSI) to ships. 

The transmissions are layered on top of SITOR collective B-mode. SITOR-B is a forward error correcting (FEC) broadcast that uses the CCIR 476 character set. NAVTEX messages are transmitted at 100 baud using FSK modulation with a frequency shift of 170 Hz.

The NavTex plugin can be used to receive and decode these messages,
the plugin will set tuning to 518KHz, the standard frequency for 
navtex messages (Another frequency for navtex messages is 490 KHz.)

-------------------------------------------------------------------------
Using the plugin
-------------------------------------------------------------------------

![overview](/navtex-plugin.png?raw=true)

First of all, ensure that:

 * Select the frequency 518KHz (selecting the plugin will set the frequence
to that.

If/when there is a transmission, this shows on the main spectrum display

Now please realize that the signal is a PSK signal with a shift (difference
b etwen mark and space) of only 170 Hz, 170 Hz on a display with a width of
62.5KHz is hard to identify. Happily enough, the tuning of the SDRplays (at least the versions I have) is precise!

SWith the default settings on the plugin, data will be shown, but in absence
of the signal, probably garbage.

The plugin as 5 controls

 * a combobox for switching the AFC on/off
   If the afc is switched on, the computed offset is shown in the right one of
the two numeric displays,

 * a combobox for switching mark and space in the decoding,

 * a combobox for 
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
