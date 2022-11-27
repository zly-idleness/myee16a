#include <Wire.h>
uint8_t drive = P2_4;       // Control output for the drive switch
uint8_t clean = P2_5;       // Control output for the clean switch
float vcc = 3.3;
uint16_t dac_bits = 4095;

#define MCP4725_I2CADDR (0x60)
#define MCP4725_CMD_WRITEDAC (0x40)    ///< Writes data to the DAC
#define MCP4725_CMD_WRITEDACEEPROM (0x60) ///< Writes data to EEPROM

String buff(20);

int f = 10;

/* The setup() function runs every time the Arduino is powered
   on or the RESET button is pressed.*/
void setup() {
  // Start a serial connection to send data to the computer
  Serial.begin(115200);
  Wire.begin();

  Serial.println("+-----Starting Capacitive Touchscreen Switch Control-------+");
  Serial.println("| Enter a voltage value between 0V and 3.3V to change Vref |");
  Serial.println("|              The default Vref value is 0.5V              |");
  Serial.println("+----------------------------------------------------------+");

  pinMode(drive, OUTPUT);
  pinMode(clean, OUTPUT);

  digitalWrite(drive, LOW);
  digitalWrite(clean, LOW);

  write_dac(620);
}

/* The loop() function is executed over and over as long as the
   MSP430 is receiving power.*/
void loop () {
  sense();
}

void write_dac( uint16_t voltage ) {
  uint8_t packet[3];
  packet[0] = MCP4725_CMD_WRITEDAC;
  packet[1] = voltage / 16;
  packet[2] = (voltage % 16) << 4;
  Wire.beginTransmission(MCP4725_I2CADDR);
  Wire.write(packet[0]);
  Wire.write(packet[1]);
  Wire.write(packet[2]);
  Wire.endTransmission();
}

void serialEvent() {
  buff = Serial.readStringUntil('\n');
  float buff_int = buff.toFloat();
  uint16_t to_write = (0x0FFF) & int((buff_int / vcc * dac_bits));
  if (buff_int != 0 && buff_int <= vcc && buff_int >= 0) {
    write_dac(to_write);
    Serial.print("Setting DAC to ");
    Serial.print(buff_int);
    Serial.print("V (");
    Serial.print(to_write);
    Serial.println(")");
  }
  else {
    Serial.println("Must be voltage between 0 and Vcc");
  }
}

/* sense() performs a full switching sequence and
  outputs the voltage reading (float).*/
void sense() {
  // STEP 1: Drain all charge
  digitalWrite(clean, HIGH);

  delayMicroseconds(f);

  // STEP 2: Stop discharging
  digitalWrite(clean, LOW);
  
  // STEP 3: Charge Ctouchscreen
  digitalWrite(drive, HIGH);

  delayMicroseconds(f);

  // STEP 4: Share charge with reference cap
  digitalWrite(drive, LOW);
  delayMicroseconds(f);
}
