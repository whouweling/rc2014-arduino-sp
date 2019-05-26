# rc2014-arduino-sp

Service Processor for RC2014: controls cpu clock, resets cpu and boots CP/M upon power-on

   Connect 4 pins in the layout below:

     - Connect RX & TC to the serial port of the RC2014
     - Connect the CLOCK pin to the clock signal on the bus
     - Connect the RESET pin to the reset signal on the bus

   Use **19200**  baud for the SP serial connection to your computer.

**Commands for the SP**:

     - double press '`' to enter the SP prompt mode
     - "clock [number]": select clock speed, first parameter switches between clockSettings
     - "slow": switch to very slow clock, use this to watch signals on the data / address bus using leds
     - "fast": switch back to normal clock setting
     - "reset": resets the CPU
     - "console": switch from SP prompt mode to the serial console
     - "step": switch to manual clock, each [enter] key steps the clock one cycle
     - "boot": reset the CPU and reboot into CP/M

**Notes**:

     - There is no serial output from the RC2014 in slow clock or step mode
     - The fastest clock is currently 614400 hz, when going faster the software serial cannot keep up. Use an Arduino with multiple serial
       ports to work around this.

**TODO**:
   
     - Connect a I2C display and use special chars to pass-through data to the display from the RC2014 serial
     - Watchdog mode
     - Persistant boot setting
     
     
**See it in action**:
  
- Booting the RC2014 using the SP: [Video1](https://www.youtube.com/watch?v=J5clVXSjsek)
- Slow mode to show the bus signals: [Video2](https://www.youtube.com/watch?v=Qgao4P8QTbw)
    
