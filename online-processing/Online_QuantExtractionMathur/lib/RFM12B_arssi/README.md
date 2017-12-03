enhanced RFM12B with ARSSI Library
----------------------------------
By Charles-Henri Hallard
<br/>
Enhanced RFM12B with Analog RSSI reading RF module library for Arduino

##License
Same as the original libraries

You are free to share and adapt. But you need to give attribution and use the same license to redistribute.

For any explanation see RFM12 module see http://www.hoperf.com/rf/fsk_module/RFM12B.htm

Code based on following datasheet http://www.hoperf.com/upload/rf/RFM12B.pdf

##Features
List of features implemented in this library:

- Module detection (can check if module is present on board or not)
- Accurate Analog RSSI measurement (need hardware hack by soldering one wire)
- Dynamic use of Hardware IRQ (D2/INT0 or D3/INT1) for Atmega328
- Dynamic use of Hardware IRQ (D10/INT0, D11/INT1 or D2/INT2) for Atmega1284
- Dynamic use of custom pin for Chip Select 
- AtMega 1284p External Interrupt compatibility (use Felix's [Mega Core][14] pin mapping for this)
- Added time-out to initialize function to avoid lockup

This is an initial release, it could contain bugs, but works fine for now. Please let me know if you find issues.

###Installation
Copy the content of this library in the "Arduino/libraries/RFM12B_arssi" folder.
<br />
To find your Arduino folder go to File>Preferences in the Arduino IDE.
<br/>
See [this tutorial][1] on Arduino libraries.
<br/>

###Possible issues
- Analog RSSI signal differs from modules (location and values)


###Sample usage
- [RFM12B_Struct_gateway_arssi][12] Receive data then read RSSI and send back to node RSSI value
- [RFM12B_Struct_node_arssi][13] Read some values, send to gateway then receive RSSI value

##Blog dedicated post
See this [post][5] for information

##Why
- I have long used Felix's RFM12B library, but I needed more features.

##Reference
Thanks to all contributors starting Jean-Claude Wippler from [jeelabs][6] for writing original Library.

- Felix Russu from [lowpowerlab][7] for providing more flexibility to it.
- Strobotics for [RFM12B hardware interface][8] with some ARSSI description.
- Marc for [Reading reading ARSSI][9] signal of RFM12B and RFM12B chipset [advanced][10] description.
- Scott Daniels for [Secret Arduino Voltmeter][11] Measure Battery Voltage.

##Misc
 See news and other projects on my [blog][4] 
 
[1]: http://learn.adafruit.com/arduino-tips-tricks-and-techniques/arduino-libraries
[4]: http://hallard.me
[5]: http://hallard.me/RFM12B_arssi-library/
[6]: http://jeelabs.org/about/
[7]: http://lowpowerlab.com/about/
[8]: http://blog.strobotics.com.au/2008/06/17/rfm12-tutorial-part2/
[9]: http://http://scurvyrat.com/2011/06/14/reading-the-arssi-pin-on-the-si4221hoperf-12b/
[10]: http://scurvyrat.com/2011/06/06/silicon-labs-chips/
[11]: http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
[12]: https://github.com/hallard/RFM12B_arssi/blob/master/Examples/RFM12B_Struct_gateway_arssi/RFM12B_Struct_gateway_arssi.ino
[13]: https://github.com/hallard/RFM12B_arssi/blob/master/Examples/RFM12B_Struct_node_arssi/RFM12B_Struct_node_arssi.ino
[14]: https://github.com/LowPowerLab/Moteino/tree/master/MEGA/Core

