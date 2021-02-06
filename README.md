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



