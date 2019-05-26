
/* Service Processor for RC2014: controls cpu clock, resets cpu and boots CP/M upon power-on

   Connect 4 pins in the layout below:

     - Connect RX & TC to the serial port of the RC2014
     - Connect the CLOCK pin to the clock signal on the bus
     - Connect the RESET pin to the reset signal on the bus

   Use 19200 baud for the SP serial connection to your computer.

   Commands for the SP:

     - double press '`' to enter the SP prompt mode
     - "clock [number]": select clock speed, first parameter switches between clockSettings
     - "slow": switch to very slow clock, use this to watch signals on the data / address bus using leds
     - "fast": switch back to normal clock setting
     - "reset": resets the CPU
     - "console": switch from SP prompt mode to the serial console
     - "step": switch to manual clock, each [enter] key steps the clock one cycle
     - "boot": reset the CPU and reboot into CP/M

   Notes:

     - There is no serial output from the RC2014 in slow clock or step mode
     - The fastest clock is currently 614400 hz, when going faster the software serial cannot keep up. Use an Arduino with multiple serial
       ports to work around this.

   TODO:
     - Connect a I2C display and use special chars to pass-through data to the display from the RC2014 serial
     - Watchdog mode
     - Persistant boot setting
*/

#include <SoftwareSerial.h>

/* To RC2014 serial */
static int RX_PIN = 10;
static int TX_PIN = 11;

/* To RC2014 Clock & Reset */
static int CLOCK_PIN = 3;
static int RESET_PIN = 7;

SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

char * version = "0.2";

int clockSettings[8] = {
  // 12, 19200, // 1228800 hz
  // 16, 14400, // 307200 hz
  25, 9600, // 614400 hz
  51, 4800, // 307200 hz
  207, 1200, // 76800 hz
};

// 19200 hz = 300 = 103

int sp_mode = 1;
int clockSpeed = 0;

char * command;
char * p;

long sp_trigger_key = 0;

void setup()
{
  delay(100);

  command = malloc(40);
  p = command;

  /* Setup serial */
  Serial.begin(19200);

  clear_screen();

  Serial.print("** SP ");
  Serial.print(version);
  Serial.println("");

  /* Enable clock */
  enable_clock();

  boot();
  prompt();

}

void clear_screen() {
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command

}

void enable_clock() {

  int timer = clockSettings[clockSpeed];
  int baudRate = clockSettings[clockSpeed + 1];

  Serial.print("** SP: CLOCK: ");
  Serial.print(timer);
  Serial.print(" / ");
  Serial.println(baudRate);

  mySerial.begin(baudRate);

  pinMode(CLOCK_PIN, OUTPUT) ;

  TCCR2B = 0x09;
  TCCR2A = 0x23;

  OCR2A = timer;
  OCR2B = 0 ;
  TCNT2 = 0 ;
}

void set_slow() {
  TCCR2B = 0x07;
  OCR2A = 255;
  mySerial.begin(100);

}


void reset_cpu() {
  /* Reset main CPU */
  pinMode(RESET_PIN, OUTPUT);
  delay(1000);
  digitalWrite(RESET_PIN, 0);
  delay(100);
  digitalWrite(RESET_PIN, 1);
  delay(500);
  Serial.println("** SP: RESET");
}

void prompt() {
  Serial.println("");
  Serial.print("SP> ");
}

void boot() {
  reset_cpu();

  Serial.println("** SP: BOOT");

  delay(400);
  mySerial.write(" ");
  delay(200);
  mySerial.write("X");
  delay(200);
  mySerial.write("Y");

  switch_console();
}

void switch_console() {
  Serial.println("** SP: CONSOLE");
  delay(1000);
  clear_screen();
  sp_mode = 0;
}


void switch_sp() {
  clear_screen();
  Serial.println("");
  Serial.println("** SP: SWITCH TO SP");
  Serial.println("");
  sp_mode = 1;
  prompt();
}

void step_clock() {

  Serial.println("** SP: STEP CLOCK [ENTER]");

  pinMode (CLOCK_PIN, OUTPUT) ;

  int sig = 0;
  while (1) {

    if (Serial.available()) {

      sig = ! sig;
      if (sig) {
        digitalWrite(CLOCK_PIN, LOW);
      } else {
        digitalWrite(CLOCK_PIN, HIGH);
      }

      *p = Serial.read();

      if (*p != '\r') {
        return;
      }
    }
  }
}

void execute(char * command) {

  Serial.println("");

  char base[20];
  char parameter[10];

  char * b = base;
  char * p = parameter;
  int pIndex = 0;
  while (*command) {
    if (*command == ' ') {
      pIndex++;
    } else {
      if (pIndex) {
        *p = *command;
        *p++;
      } else {
        *b = *command;
        *b ++;
      }
    }
    command++;
  }

  *b = '\0';
  *p = '\0';

  if (strcmp(base, "reset") == 0) {
    reset_cpu();
    switch_console();
    return;
  }

  if (strcmp(base, "boot") == 0) {
    boot();
    sp_mode = 0;
    return;
  }

  if (strcmp(base, "console") == 0 
      || strcmp(base, "exit") == 0) {
    switch_console();
    return;
  }

  if (strcmp(base, "slow") == 0) {
    set_slow();
    switch_console();
    return;
  }

  if (strcmp(base, "fast") == 0) {
    enable_clock();
    switch_console();
    return;
  }

  if (strcmp(base, "step") == 0) {
    step_clock();
    return;
  }

  if (strcmp(base, "clock") == 0) {
    clockSpeed = atoi(parameter);
    if (clockSpeed < 0) {
      clockSpeed = 0;
    }
    if (clockSpeed > 4) {
      clockSpeed = 4;
    }
    enable_clock();
    return;
  }

  Serial.print(base);
  Serial.println("?");

}

void loop() {
  
  if (sp_mode) {

    /* SP prompt mode */
    if (Serial.available()) {
      *p = Serial.read();
      Serial.print(*p);
      if (*p == '\r') {
        *p = '\0';
        execute(command);
        if (sp_mode) {
          prompt();
        }
        p = command;
        *p = '\0';
      } else {
        *p ++; *p = '\0';
      }
    }

  } else {

    /* Pass through mode */
    if (mySerial.available()) {
      Serial.write(mySerial.read());
    }
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '~') {
        if(millis() - sp_trigger_key < 500) {
           switch_sp();          
        }
        sp_trigger_key = millis();
      } else {
        mySerial.write(c);
      }
    }
  }
}
