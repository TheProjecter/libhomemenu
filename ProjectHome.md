A library that allows wii homebrew coders to quickly implement a decent-looking HOME menu.

It is graphics library independent, so there is hardly any set up, and generates events callbacks functions can be associated with.


### Download Notice ###
Until I release v1.0, I will mark a download deprecated if it doesn't not reflect the content of the SVN.  You can still access it searching deprecated downloads.  I strongly encourage you, however, to work form the svn.

### To do list: ###
  * ~~sound (to be added shortly).~~
  * ~~rumble (to be added shortly).~~
  * battery level (to be added once next wpad is released)
  * wiimote config panel (to be added in the distant future)
  * confirmation dialogs (to be added shortly)
  * ~~Hotspot activation animations.~~
  * ~~Hotspot activation sounds~~.
  * Confirmation dialog.
  * Improved callback interface.
  * convert textures to compressed textures.
  * ~~exit on the correct frame buffer.~~
  * ~~convert to C.~~
  * test widescreen and PAL resolutions
  * fix button hotspots in widescreen resolutions (should be very quick)
  * documentation
  * Add configuration for developers:
    * Specify underlying graphics approach. GRRLIB|LWS|GX|other (since we apparently need more graphics details than I anticipated)
    * other minor config such as buttons and sound