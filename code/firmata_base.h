/*
  Firmata is a generic protocol for communicating with microcontrollers
  from software on a host computer. It is intended to work with
  any host computer software package.

  To download a host software package, please clink on the following link
  to open the list of Firmata client libraries your default browser.

  https://github.com/firmata/arduino#firmata-client-libraries

  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2009-2015 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated by Jeff Hoefs: November 7th, 2015
*/

extern "C" {
  extern uint32_t __koov_data_start__;
  extern uint32_t __btpin_data_start__;
  extern uint32_t __btpin_data_end__;
};

namespace firmata_base {

#define I2C_WRITE                   B00000000
#define I2C_READ                    B00001000
#define I2C_READ_CONTINUOUSLY       B00010000
#define I2C_STOP_READING            B00011000
#define I2C_READ_WRITE_MODE_MASK    B00011000
#define I2C_10BIT_ADDRESS_MODE_MASK B00100000
#define I2C_MAX_QUERIES             8
#define I2C_REGISTER_NOT_SPECIFIED  -1

// the minimum interval for sampling analog input
#define MINIMUM_SAMPLING_INTERVAL   1

struct flash_state {
  uint32_t fs_start;
  uint32_t fs_end;
  uint32_t fs_offset;
  uint16_t fs_value;
  byte fs_shift;
  byte fs_flags;
#define FS_ESCAPED 0x01		// previous char was escape char
#define FS_RAW 0x02		// input is raw (no esacpe char)
} flash_state;
static void koov_sysex(byte argc, byte *argv);
void reportAnalogCallback(byte analogPin, int value);
void sysexCallback(byte command, byte argc, byte *argv);
void enableI2CPins();
void disableI2CPins();

#undef DEBUG_USB

/*==============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/

/* analog inputs */
int analogInputsToReport = 0; // bitwise array to store pin reporting

/* digital input ports */
byte reportPINs[TOTAL_PORTS];       // 1 = report this port, 0 = silence
byte previousPINs[TOTAL_PORTS];     // previous 8 bits sent

/* pins configuration */
byte pinConfig[TOTAL_PINS];         // configuration of every pin
byte portConfigInputs[TOTAL_PORTS]; // each bit: 1 = pin in INPUT, 0 = anything else
int pinState[TOTAL_PINS];           // any value that has been written

/* timer variables */
unsigned long currentMillis;        // store the current value from millis()
unsigned long previousMillis;       // for comparison with currentMillis
unsigned int samplingInterval = 19; // how often to run the main loop (in ms)

/* i2c data */
struct i2c_device_info {
  byte addr;
  int reg;
  byte bytes;
};

/* for i2c read continuous more */
i2c_device_info query[I2C_MAX_QUERIES];

byte i2cRxData[32];
boolean isI2CEnabled = false;
signed char queryIndex = -1;
// default delay time between i2c read request and Wire.requestFrom()
unsigned int i2cReadDelayTime = 0;

Servo servos[MAX_SERVOS];
byte servoPinMap[TOTAL_PINS];
byte detachedServos[MAX_SERVOS];
byte detachedServoCount = 0;
byte servoCount = 0;

boolean isResetting = false;

/* utility functions */
void wireWrite(byte data)
{
#if ARDUINO >= 100
  Wire.write((byte)data);
#else
  Wire.send(data);
#endif
}

byte wireRead(void)
{
#if ARDUINO >= 100
  return Wire.read();
#else
  return Wire.receive();
#endif
}

/*==============================================================================
 * FUNCTIONS
 *============================================================================*/

void attachServo(byte pin, int minPulse, int maxPulse)
{
  if (servoCount < MAX_SERVOS) {
    // reuse indexes of detached servos until all have been reallocated
    if (detachedServoCount > 0) {
      servoPinMap[pin] = detachedServos[detachedServoCount - 1];
      if (detachedServoCount > 0) detachedServoCount--;
    } else {
      servoPinMap[pin] = servoCount;
      servoCount++;
    }
    if (minPulse > 0 && maxPulse > 0) {
      servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin), minPulse, maxPulse);
    } else {
      servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin));
    }
  } else {
    Firmata.sendString("Max servos attached");
  }
}

void detachServo(byte pin)
{
  servos[servoPinMap[pin]].detach();
  // if we're detaching the last servo, decrement the count
  // otherwise store the index of the detached servo
  if (servoPinMap[pin] == servoCount && servoCount > 0) {
    servoCount--;
  } else if (servoCount > 0) {
    // keep track of detached servos because we want to reuse their indexes
    // before incrementing the count of attached servos
    detachedServoCount++;
    detachedServos[detachedServoCount - 1] = servoPinMap[pin];
  }

  servoPinMap[pin] = 255;
}

void detachServoMaybe(byte pin)
{
  if (servoPinMap[pin] < MAX_SERVOS && servos[servoPinMap[pin]].attached()) {
    detachServo(pin);
  }
}

void readAndReportData(byte address, int theRegister, byte numBytes) {
  // allow I2C requests that don't require a register read
  // for example, some devices using an interrupt pin to signify new data available
  // do not always require the register read so upon interrupt you call Wire.requestFrom()
  if (theRegister != I2C_REGISTER_NOT_SPECIFIED) {
    Wire.beginTransmission(address);
    wireWrite((byte)theRegister);
    Wire.endTransmission();
    // do not set a value of 0
    if (i2cReadDelayTime > 0) {
      // delay is necessary for some devices such as WiiNunchuck
      delayMicroseconds(i2cReadDelayTime);
    }
  } else {
    theRegister = 0;  // fill the register with a dummy value
  }

  Wire.requestFrom(address, numBytes);  // all bytes are returned in requestFrom

  // check to be sure correct number of bytes were returned by slave
  if (numBytes < Wire.available()) {
    Firmata.sendString("I2C: Too many bytes received");
  } else if (numBytes > Wire.available()) {
    Firmata.sendString("I2C: Too few bytes received");
  }

  i2cRxData[0] = address;
  i2cRxData[1] = theRegister;

  for (int i = 0; i < numBytes && Wire.available(); i++) {
    i2cRxData[2 + i] = wireRead();
  }

  // send slave address, register and received bytes
  Firmata.sendSysex(SYSEX_I2C_REPLY, numBytes + 2, i2cRxData);
}

void outputPort(byte portNumber, byte portValue, byte forceSend)
{
  // pins not configured as INPUT are cleared to zeros
  portValue = portValue & portConfigInputs[portNumber];
  // only send if the value is different than previously sent
  if (forceSend || previousPINs[portNumber] != portValue) {
    Firmata.sendDigitalPort(portNumber, portValue);
    previousPINs[portNumber] = portValue;
  }
}

/* -----------------------------------------------------------------------------
 * check all the active digital inputs for change of state, then add any events
 * to the Serial output queue using Serial.print() */
void checkDigitalInputs(void)
{
#if 0
  if (reportPINs[0]) SerialUSB.print("0");
  if (reportPINs[1]) SerialUSB.print("1");
  if (reportPINs[2]) SerialUSB.print("2");
#endif
  /* Using non-looping code allows constants to be given to readPort().
   * The compiler will apply substantial optimizations if the inputs
   * to readPort() are compile-time constants. */
  if (TOTAL_PORTS > 0 && reportPINs[0]) outputPort(0, readPort(0, portConfigInputs[0]), false);
  if (TOTAL_PORTS > 1 && reportPINs[1]) outputPort(1, readPort(1, portConfigInputs[1]), false);
  if (TOTAL_PORTS > 2 && reportPINs[2]) outputPort(2, readPort(2, portConfigInputs[2]), false);
  if (TOTAL_PORTS > 3 && reportPINs[3]) outputPort(3, readPort(3, portConfigInputs[3]), false);
  if (TOTAL_PORTS > 4 && reportPINs[4]) outputPort(4, readPort(4, portConfigInputs[4]), false);
  if (TOTAL_PORTS > 5 && reportPINs[5]) outputPort(5, readPort(5, portConfigInputs[5]), false);
  if (TOTAL_PORTS > 6 && reportPINs[6]) outputPort(6, readPort(6, portConfigInputs[6]), false);
  if (TOTAL_PORTS > 7 && reportPINs[7]) outputPort(7, readPort(7, portConfigInputs[7]), false);
  if (TOTAL_PORTS > 8 && reportPINs[8]) outputPort(8, readPort(8, portConfigInputs[8]), false);
  if (TOTAL_PORTS > 9 && reportPINs[9]) outputPort(9, readPort(9, portConfigInputs[9]), false);
  if (TOTAL_PORTS > 10 && reportPINs[10]) outputPort(10, readPort(10, portConfigInputs[10]), false);
  if (TOTAL_PORTS > 11 && reportPINs[11]) outputPort(11, readPort(11, portConfigInputs[11]), false);
  if (TOTAL_PORTS > 12 && reportPINs[12]) outputPort(12, readPort(12, portConfigInputs[12]), false);
  if (TOTAL_PORTS > 13 && reportPINs[13]) outputPort(13, readPort(13, portConfigInputs[13]), false);
  if (TOTAL_PORTS > 14 && reportPINs[14]) outputPort(14, readPort(14, portConfigInputs[14]), false);
  if (TOTAL_PORTS > 15 && reportPINs[15]) outputPort(15, readPort(15, portConfigInputs[15]), false);
}

// -----------------------------------------------------------------------------
/* sets the pin mode to the correct state and sets the relevant bits in the
 * two bit-arrays that track Digital I/O and PWM status
 */
void setPinModeCallback(byte pin, int mode)
{
  if (pinConfig[pin] == PIN_MODE_IGNORE)
    return;

#ifdef DEBUG_USB
  SerialUSB.print("set pin mode: pin ");
  SerialUSB.print(pin, DEC);
  SerialUSB.print(" mode ");
  SerialUSB.print(mode, HEX);
  SerialUSB.println("");
#endif
  if (pinConfig[pin] == PIN_MODE_I2C && isI2CEnabled && mode != PIN_MODE_I2C) {
    // disable i2c so pins can be used for other functions
    // the following if statements should reconfigure the pins properly
    disableI2CPins();
  }
  if (IS_PIN_DIGITAL(pin) && mode != PIN_MODE_SERVO) {
    if (servoPinMap[pin] < MAX_SERVOS && servos[servoPinMap[pin]].attached()) {
      detachServo(pin);
    }
  }
  if (IS_PIN_ANALOG(pin)) {
    reportAnalogCallback(PIN_TO_ANALOG(pin), mode == PIN_MODE_ANALOG ? 1 : 0); // turn on/off reporting
  }
  if (IS_PIN_DIGITAL(pin)) {
    if (mode == INPUT || mode == PIN_MODE_PULLUP) {
      portConfigInputs[pin / 8] |= (1 << (pin & 7));
    } else {
      portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
    }
  }
  pinState[pin] = 0;
  switch (mode) {
    case PIN_MODE_ANALOG:
      if (IS_PIN_ANALOG(pin)) {
        if (IS_PIN_DIGITAL(pin)) {
          pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
          // deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
          digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
        }
        pinConfig[pin] = PIN_MODE_ANALOG;
      }
      break;
    case INPUT:
      if (IS_PIN_DIGITAL(pin)) {
        pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
        // deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
        digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
        pinConfig[pin] = INPUT;
      }
      break;
    case PIN_MODE_PULLUP:
      if (IS_PIN_DIGITAL(pin)) {
        pinMode(PIN_TO_DIGITAL(pin), INPUT_PULLUP);
        pinConfig[pin] = PIN_MODE_PULLUP;
        pinState[pin] = 1;
      }
      break;
    case OUTPUT:
      if (IS_PIN_DIGITAL(pin)) {
        digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable PWM
        pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
        pinConfig[pin] = OUTPUT;
      }
      break;
    case PIN_MODE_PWM:
      if (IS_PIN_PWM(pin)) {
        pinMode(PIN_TO_PWM(pin), OUTPUT);
        analogWrite(PIN_TO_PWM(pin), 0);
        pinConfig[pin] = PIN_MODE_PWM;
      }
      break;
    case PIN_MODE_SERVO:
      if (IS_PIN_DIGITAL(pin)) {
        pinConfig[pin] = PIN_MODE_SERVO;
        if (servoPinMap[pin] == 255 || !servos[servoPinMap[pin]].attached()) {
          // pass -1 for min and max pulse values to use default values set
          // by Servo library
          attachServo(pin, -1, -1);
        }
      }
      break;
    case PIN_MODE_I2C:
      if (IS_PIN_I2C(pin)) {
        // mark the pin as i2c
        // the user must call I2C_CONFIG to enable I2C for a device
        pinConfig[pin] = PIN_MODE_I2C;
      }
      break;
    default:
      Firmata.sendString("Unknown pin mode"); // TODO: put error msgs in EEPROM
  }
  // TODO: save status to EEPROM here, if changed
}

/*
 * Sets the value of an individual pin. Useful if you want to set a pin value but
 * are not tracking the digital port state.
 * Can only be used on pins configured as OUTPUT.
 * Cannot be used to enable pull-ups on Digital INPUT pins.
 */
void setPinValueCallback(byte pin, int value)
{
  if (pin < TOTAL_PINS && IS_PIN_DIGITAL(pin)) {
    if (pinConfig[pin] == OUTPUT) {
      pinState[pin] = value;
      digitalWrite(PIN_TO_DIGITAL(pin), value);
    }
  }
}

void analogWriteCallback(byte pin, int value)
{
#ifdef DEBUG_USB
  SerialUSB.print("analog write: pin ");
  SerialUSB.print(pin, DEC);
  SerialUSB.print(" value 0x");
  SerialUSB.print(value, HEX);
  SerialUSB.print(" isPWN ");
  SerialUSB.print(IS_PIN_PWM(pin), DEC);
  SerialUSB.print(" pin ");
  SerialUSB.print(PIN_TO_PWM(pin), DEC);
  SerialUSB.println("");
#endif
  if (pin < TOTAL_PINS) {
    switch (pinConfig[pin]) {
      case PIN_MODE_SERVO:
        if (IS_PIN_DIGITAL(pin))
          servos[servoPinMap[pin]].write(value);
        pinState[pin] = value;
        break;
      case PIN_MODE_PWM:
        if (IS_PIN_PWM(pin))
          analogWrite(PIN_TO_PWM(pin), value);
        pinState[pin] = value;
        break;
    }
  }
}

void digitalWriteCallback(byte port, int value)
{
  byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

#ifdef DEBUG_USB
  SerialUSB.print("digital write: port ");
  SerialUSB.print(port, DEC);
  SerialUSB.print(" value ");
  SerialUSB.print(value, HEX);
  SerialUSB.println("");
  SerialUSB.print("pinConfig[] = {");
  for (int i = 0; i < TOTAL_PINS; i++) {
    SerialUSB.print(pinConfig[i], DEC);
    SerialUSB.print(",");
  }
  SerialUSB.println("}");
  SerialUSB.print("pinState[] = {");
  for (int i = 0; i < TOTAL_PINS; i++) {
    SerialUSB.print(pinState[i], DEC);
    SerialUSB.print(",");
  }
  SerialUSB.println("}");
#endif
  if (port < TOTAL_PORTS) {
    // create a mask of the pins on this port that are writable.
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
      // do not disturb non-digital pins (eg, Rx & Tx)
      if (IS_PIN_DIGITAL(pin)) {
        // do not touch pins in PWM, ANALOG, SERVO or other modes
        if (pinConfig[pin] == OUTPUT || pinConfig[pin] == INPUT) {
          pinValue = ((byte)value & mask) ? 1 : 0;
          if (pinConfig[pin] == OUTPUT) {
            pinWriteMask |= mask;
          } else if (pinConfig[pin] == INPUT && pinValue == 1 && pinState[pin] != 1) {
            // only handle INPUT here for backwards compatibility
#if ARDUINO > 100
            pinMode(pin, INPUT_PULLUP);
#else
            // only write to the INPUT pin to enable pullups if Arduino v1.0.0 or earlier
            pinWriteMask |= mask;
#endif
          }
          pinState[pin] = pinValue;
        }
      }
      mask = mask << 1;
    }

#ifdef DEBUG_USB
    SerialUSB.print("write port: port ");
    SerialUSB.print(port, DEC);
    SerialUSB.print(" mask ");
    SerialUSB.print(pinWriteMask, HEX);
    SerialUSB.println("");
#endif
    writePort(port, (byte)value, pinWriteMask);
  }
}


// -----------------------------------------------------------------------------
/* sets bits in a bit array (int) to toggle the reporting of the analogIns
 */
//void FirmataClass::setAnalogPinReporting(byte pin, byte state) {
//}
void reportAnalogCallback(byte analogPin, int value)
{
  if (analogPin < TOTAL_ANALOG_PINS) {
    if (value == 0) {
      analogInputsToReport = analogInputsToReport & ~ (1 << analogPin);
    } else {
      analogInputsToReport = analogInputsToReport | (1 << analogPin);
      // prevent during system reset or all analog pin values will be reported
      // which may report noise for unconnected analog pins
      if (!isResetting) {
        // Send pin value immediately. This is helpful when connected via
        // ethernet, wi-fi or bluetooth so pin states can be known upon
        // reconnecting.
        Firmata.sendAnalog(analogPin, analogRead(PIN_TO_ANALOG(analogPin)));
      }
    }
  }
  // TODO: save status to EEPROM here, if changed
}

void reportDigitalCallback(byte port, int value)
{
#ifdef DEBUG_USB
  SerialUSB.print("reportDigitalCallback port ");
  SerialUSB.print(port, DEC);
  SerialUSB.print(" value ");
  SerialUSB.print(value, HEX);
  SerialUSB.println("");
#endif
  if (port < TOTAL_PORTS) {
    reportPINs[port] = (byte)value;
    // Send port value immediately. This is helpful when connected via
    // ethernet, wi-fi or bluetooth so pin states can be known upon
    // reconnecting.
    if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
  }
  // do not disable analog reporting on these 8 pins, to allow some
  // pins used for digital, others analog.  Instead, allow both types
  // of reporting to be enabled, but check if the pin is configured
  // as analog when sampling the analog inputs.  Likewise, while
  // scanning digital pins, portConfigInputs will mask off values from any
  // pins configured as analog
}

/*==============================================================================
 * SYSEX-BASED commands
 *============================================================================*/

static void bts01_write(const String &s, void *args);
static bool bts01_cmd(const char *cmd,
		      unsigned int timeout = 5000,
		      void (*handler)(const String &, void *) = 0,
		      void *args = 0);

void sysexCallback(byte command, byte argc, byte *argv)
{
  byte mode;
  byte slaveAddress;
  byte data;
  int slaveRegister;
  unsigned int delayTime;

  switch (command) {
    case I2C_REQUEST:
      mode = argv[1] & I2C_READ_WRITE_MODE_MASK;
      if (argv[1] & I2C_10BIT_ADDRESS_MODE_MASK) {
        Firmata.sendString("10-bit addressing not supported");
        return;
      }
      else {
        slaveAddress = argv[0];
      }

      switch (mode) {
        case I2C_WRITE:
          Wire.beginTransmission(slaveAddress);
          for (byte i = 2; i < argc; i += 2) {
            data = argv[i] + (argv[i + 1] << 7);
            wireWrite(data);
          }
          Wire.endTransmission();
          delayMicroseconds(70);
          break;
        case I2C_READ:
          if (argc == 6) {
            // a slave register is specified
            slaveRegister = argv[2] + (argv[3] << 7);
            data = argv[4] + (argv[5] << 7);  // bytes to read
          }
          else {
            // a slave register is NOT specified
            slaveRegister = I2C_REGISTER_NOT_SPECIFIED;
            data = argv[2] + (argv[3] << 7);  // bytes to read
          }
          readAndReportData(slaveAddress, (int)slaveRegister, data);
          break;
        case I2C_READ_CONTINUOUSLY:
          if ((queryIndex + 1) >= I2C_MAX_QUERIES) {
            // too many queries, just ignore
            Firmata.sendString("too many queries");
            break;
          }
          if (argc == 6) {
            // a slave register is specified
            slaveRegister = argv[2] + (argv[3] << 7);
            data = argv[4] + (argv[5] << 7);  // bytes to read
          }
          else {
            // a slave register is NOT specified
            slaveRegister = (int)I2C_REGISTER_NOT_SPECIFIED;
            data = argv[2] + (argv[3] << 7);  // bytes to read
          }
          queryIndex++;
          query[queryIndex].addr = slaveAddress;
          query[queryIndex].reg = slaveRegister;
          query[queryIndex].bytes = data;
          break;
        case I2C_STOP_READING:
          byte queryIndexToSkip;
          // if read continuous mode is enabled for only 1 i2c device, disable
          // read continuous reporting for that device
          if (queryIndex <= 0) {
            queryIndex = -1;
          } else {
            // if read continuous mode is enabled for multiple devices,
            // determine which device to stop reading and remove it's data from
            // the array, shifiting other array data to fill the space
            for (byte i = 0; i < queryIndex + 1; i++) {
              if (query[i].addr == slaveAddress) {
                queryIndexToSkip = i;
                break;
              }
            }

            for (byte i = queryIndexToSkip; i < queryIndex + 1; i++) {
              if (i < I2C_MAX_QUERIES) {
                query[i].addr = query[i + 1].addr;
                query[i].reg = query[i + 1].reg;
                query[i].bytes = query[i + 1].bytes;
              }
            }
            queryIndex--;
          }
          break;
        default:
          break;
      }
      break;
    case I2C_CONFIG:
      delayTime = (argv[0] + (argv[1] << 7));

      if (delayTime > 0) {
        i2cReadDelayTime = delayTime;
      }

      if (!isI2CEnabled) {
        enableI2CPins();
      }

      break;
    case SERVO_CONFIG:
      if (argc > 4) {
        // these vars are here for clarity, they'll optimized away by the compiler
        byte pin = argv[0];
        int minPulse = argv[1] + (argv[2] << 7);
        int maxPulse = argv[3] + (argv[4] << 7);

        if (IS_PIN_DIGITAL(pin)) {
	  detachServoMaybe(pin);
          attachServo(pin, minPulse, maxPulse);
          setPinModeCallback(pin, PIN_MODE_SERVO);
        }
      }
      break;
    case SAMPLING_INTERVAL:
      if (argc > 1) {
        samplingInterval = argv[0] + (argv[1] << 7);
        if (samplingInterval < MINIMUM_SAMPLING_INTERVAL) {
          samplingInterval = MINIMUM_SAMPLING_INTERVAL;
        }
      } else {
        //Firmata.sendString("Not enough data");
      }
      break;
    case EXTENDED_ANALOG:
      if (argc > 1) {
        int val = argv[1];
        if (argc > 2) val |= (argv[2] << 7);
        if (argc > 3) val |= (argv[3] << 14);
        analogWriteCallback(argv[0], val);
      }
      break;
    case CAPABILITY_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(CAPABILITY_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        if (IS_PIN_DIGITAL(pin)) {
          Firmata.write((byte)INPUT);
          Firmata.write(1);
          Firmata.write((byte)PIN_MODE_PULLUP);
          Firmata.write(1);
          Firmata.write((byte)OUTPUT);
          Firmata.write(1);
        }
        if (IS_PIN_ANALOG(pin)) {
          Firmata.write(PIN_MODE_ANALOG);
          Firmata.write(10); // 10 = 10-bit resolution
        }
        if (IS_PIN_PWM(pin)) {
          Firmata.write(PIN_MODE_PWM);
          Firmata.write(8); // 8 = 8-bit resolution
        }
        if (IS_PIN_DIGITAL(pin)) {
          Firmata.write(PIN_MODE_SERVO);
          Firmata.write(14);
        }
        if (IS_PIN_I2C(pin)) {
          Firmata.write(PIN_MODE_I2C);
          Firmata.write(1);  // TODO: could assign a number to map to SCL or SDA
        }
        Firmata.write(127);
      }
      Firmata.write(END_SYSEX);
      break;
    case PIN_STATE_QUERY:
      if (argc > 0) {
        byte pin = argv[0];
        Firmata.write(START_SYSEX);
        Firmata.write(PIN_STATE_RESPONSE);
        Firmata.write(pin);
        if (pin < TOTAL_PINS) {
          Firmata.write((byte)pinConfig[pin]);
          Firmata.write((byte)pinState[pin] & 0x7F);
          if (pinState[pin] & 0xFF80) Firmata.write((byte)(pinState[pin] >> 7) & 0x7F);
          if (pinState[pin] & 0xC000) Firmata.write((byte)(pinState[pin] >> 14) & 0x7F);
        }
        Firmata.write(END_SYSEX);
      }
      break;
    case ANALOG_MAPPING_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(ANALOG_MAPPING_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
      }
      Firmata.write(END_SYSEX);
      break;
    case 0x0f:
      {
	int pin = argv[0];
	int mode = argv[1];
	int freq = argv[2];

	if (IS_PIN_DIGITAL(pin)) {
	  BUZZER_CONTROL(PIN_TO_DIGITAL(pin), mode ? HIGH : LOW, freq);
	}
      }
      break;
    case 0x0e:			/* koov extension */
      koov_sysex(argc, argv);
      break;
  }
}

void enableI2CPins()
{
  byte i;
  // is there a faster way to do this? would probaby require importing
  // Arduino.h to get SCL and SDA pins
  for (i = 0; i < TOTAL_PINS; i++) {
    if (IS_PIN_I2C(i)) {
      // mark pins as i2c so they are ignore in non i2c data requests
      setPinModeCallback(i, PIN_MODE_I2C);
    }
  }

  isI2CEnabled = true;

  Wire.begin();
}

/* disable the i2c pins so they can be used for other functions */
void disableI2CPins() {
  isI2CEnabled = false;
  // disable read continuous mode for all devices
  queryIndex = -1;
}

/*==============================================================================
 * SETUP()
 *============================================================================*/

void systemResetCallback()
{
  isResetting = true;

  BUZZER_INIT();

  // initialize a defalt state
  // TODO: option to load config from EEPROM instead of default

  if (isI2CEnabled) {
    disableI2CPins();
  }

  for (byte i = 0; i < TOTAL_PORTS; i++) {
    reportPINs[i] = false;    // by default, reporting off
    portConfigInputs[i] = 0;  // until activated
    previousPINs[i] = 0;
  }

  for (byte i = 0; i < TOTAL_PINS; i++) {
    // pins with analog capability default to analog input
    // otherwise, pins default to digital output
    if (IS_PIN_ANALOG(i)) {
      // turns off pullup, configures everything
      setPinModeCallback(i, PIN_MODE_ANALOG);
    } else if (IS_PIN_DIGITAL(i)) {
      // sets the output to 0, configures portConfigInputs
      setPinModeCallback(i, OUTPUT);
    }

    servoPinMap[i] = 255;
  }
  // by default, do not report any analog inputs
  analogInputsToReport = 0;

  detachedServoCount = 0;
  servoCount = 0;

  /* send digital inputs to set the initial state on the host computer,
   * since once in the loop(), this firmware will only send on change */
  /*
  TODO: this can never execute, since no pins default to digital input
        but it will be needed when/if we support EEPROM stored config
  for (byte i=0; i < TOTAL_PORTS; i++) {
    outputPort(i, readPort(i, portConfigInputs[i]), true);
  }
  */
  INIT_OUTPUTS();
  isResetting = false;
}

static Stream &DebugStream = SerialUSB;
//static Stream &DebugStream = Serial;
class NullStream: public Stream {
public:
  int available() { return false; }
  int read() { return -1; }
  int peek() { return -1; }
  void flush() { return; }
  size_t write(uint8_t c) { return 0; }
} nullStream;

class DualStream: public Stream {
  Stream *serial;
  unsigned long current;

  void decideSerial() {
    int avail = Serial.available();
    if (avail) {
      serial = &Serial;
      return;
    }
    avail = SerialUSB.available();
    if (avail) {
      serial = &SerialUSB;
      return;
    }
  }
  Stream &currentSerial() {
    if (serial == 0) {
      decideSerial();
    }
    return serial ? *serial : nullStream;
  }
  public:
  DualStream(): serial(0), current(0) {}
  int available() {
    int avail = currentSerial().available();

    if (avail) {
      current = millis();
    } else {
      if (millis() - current > 10 * 1000)
	serial = 0;
    }
    return avail;
  }
  int read() {
    static int count = 0;
    int c = currentSerial().read();
#if 1
#else
    //DebugStream.write("r: ");
    DebugStream.write("r");
    DebugStream.print(count++, HEX);
    DebugStream.print(c, HEX);
    //DebugStream.write("\r\n");
    DebugStream.write("\a");
#endif
    return c;
  }
  int peek() { return currentSerial().peek(); }
  void flush() { currentSerial().flush(); }
  size_t write(uint8_t c) {
#if 1
    delay(5);
#else
    //DebugStream.write("w: ");
    DebugStream.write("w");
    DebugStream.print(c, HEX);
    //DebugStream.write("\r\n");
    DebugStream.write("\a");
    DebugStream.flush();
#endif
#if 0
    if (serial == 0) {
      DebugStream.write("skip\r\n");
      return 0;
    }
#endif
    return currentSerial().write(c);
  }

  enum {
    NOT_CONNECTED,
    USB_CONNECTED,
    BLE_CONNECTED
  };
  int connectMode() const {
    if (serial == &Serial)
      return BLE_CONNECTED;
    if (serial == &SerialUSB)
      return USB_CONNECTED;
    return NOT_CONNECTED;
  }
} dualStream;

static void
drain(Stream &s)
{
  while (s.available()) {
    int c = s.read();
  }
}

static bool
findString(const String &str, const char key[])
{
  size_t len = strlen(key);

  for (int i = 0; i < str.length() - len; i++) {
    int j = 0;
    for (; j < len; j++) {
      if (str[i + j] != key[j])
        break;
    }
    if (j == len)
      return true;
  }
  return false;
}

static bool enableFirmata = false;
static bool setupFirmata = true;
static void periodc_jobs();

static void enter_firmata();
static void leave_firmata();

/*
 * True if need to bail out.
 */
static bool
check_intr()
{
  if (dualStream.available()) {
    enter_firmata();
    return true;
  }
  periodc_jobs();
  return false;
}

#define CHECK_INTR(x)				\
  do {						\
    if (firmata_base::check_intr()) {		\
      return x;					\
    }						\
  } while (0)

void FirmataSetup()
{
  Firmata.setFirmwareNameAndVersion(KOOV_VERSION, FIRMATA_MAJOR_VERSION,
				    FIRMATA_MINOR_VERSION);

  Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
  Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
  Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
  Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
  Firmata.attach(SET_PIN_MODE, setPinModeCallback);
  Firmata.attach(SET_DIGITAL_PIN_VALUE, setPinValueCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.attach(SYSTEM_RESET, systemResetCallback);

  Firmata.begin(dualStream);
  // then comment out or remove lines 701 - 704 below

#if 0
  Firmata.begin(57600);
#endif
#if 0
  while (!SerialUSB) {
    ; // wait for serial port to connect. Only needed for ATmega32u4-based boards (Leonardo, etc).
  }
#endif
  //SerialUSB.println("reset");
  //pinConfig[0] = pinConfig[1] = PIN_MODE_IGNORE;
  systemResetCallback();  // reset to default config
  //SerialUSB.println("ready");
  analogReference(AR_DEFAULT);
}

void FirmataLoop()
{
  byte pin, analogPin;

  /* DIGITALREAD - as fast as possible, check for changes and output them to the
   * FTDI buffer using Serial.print()  */
#if 1
  checkDigitalInputs();
#endif
  //Serial.println("done digital input");

  /* STREAMREAD - processing incoming messagse as soon as possible, while still
   * checking digital inputs.  */
  while (Firmata.available())
    Firmata.processInput();
  //Serial.println("done firmata");

  // TODO - ensure that Stream buffer doesn't go over 60 bytes

  currentMillis = millis();
  if (currentMillis - previousMillis > samplingInterval) {
    previousMillis += samplingInterval;
    /* ANALOGREAD - do all analogReads() at the configured sampling interval */
    for (pin = 0; pin < TOTAL_PINS; pin++) {
      if (IS_PIN_ANALOG(pin) && pinConfig[pin] == PIN_MODE_ANALOG) {
        analogPin = PIN_TO_ANALOG(pin);
        if (analogInputsToReport & (1 << analogPin)) {
          Firmata.sendAnalog(analogPin, analogRead(PIN_TO_ANALOG(analogPin)));
        }
      }
    }
    // report i2c data for all device with read continuous mode enabled
    if (queryIndex > -1) {
      for (byte i = 0; i < queryIndex + 1; i++) {
        readAndReportData(query[i].addr, query[i].reg, query[i].bytes);
      }
    }
  }
}

#define USE_BLE
static bool bts01_failure = false;
static void
bts01_reset()
{
  /*
   * Reset BTS01.  Pin 43 is PA13.
   */
  pinMode(43, OUTPUT);
  digitalWrite(43, LOW);
  delay(1000);
  digitalWrite(43, HIGH);
  delay(10);
  bts01_failure = false;
}

static bool
bts01_cmd(const char *cmd, unsigned int timeout,
	  void (*handler)(const String &, void *), void *args)
{
  int done = 0;
  bool retval = false;
  unsigned int start = millis();
  static const char *const terminal[] = {
    "OK\r\n",
    "ACK\r\n",
    "ERROR="
  };

  drain(Serial);
  Serial.print(cmd);
  do {
    String str = Serial.readString();
    if (handler)
      (*handler)(str, args);
    int i = 0;
    for (; i < sizeof(terminal) / sizeof(terminal[0]); i++) {
      done = findString(str, terminal[i]);
      if (done) {
	if (i == 2) {
	  bts01_failure = bts01_failure || true;
	  retval = false;
	} else
	  retval = true;
	break;
      }
    }
    if (!done) {
      if (millis() - start > timeout)
	return retval;
      delay(10);
    }
  } while (!done && Serial.available());
  return retval;
}

static void
bts01_write(const String &s, void *args)
{
  const unsigned int len = s.length();

  for (int i = 0; i < len; i++)
    Firmata.write(s[i]);
}

static bool
bts01_rvn()
{

  // response is \r\nVN=x.yz\r\n\r\nOK\r\n
  return bts01_cmd("AT+RVN\r");
}


static bool
bts01_dbi()
{

  return bts01_cmd("AT+DBI=ALL\r");
}

static bool
bts01_ccp()
{

  return bts01_cmd("AT+CCP=0010,0028,0001,0190\r");
}

static bool
bts01_sbo()
{

  return bts01_cmd("AT+SBO\r");
}

/*
 * Write single byte into flash.
 */
#define NVM_MEMORY        ((volatile uint16_t *)FLASH_ADDR)
static void
nvm_write(uint16_t v)
{
  /* Touch NVM only when address is higher than koov_data section */
  if (flash_state.fs_start <= flash_state.fs_offset &&
      flash_state.fs_offset < flash_state.fs_end)
    NVM_MEMORY[flash_state.fs_offset / 2] = v;
  flash_state.fs_offset += 2;
}

static void
flash_write(byte cc)
{

#define ESCAPE_CHAR 0x7f
  if (flash_state.fs_flags & FS_ESCAPED) {
    cc = cc ? END_SYSEX : ESCAPE_CHAR;
    flash_state.fs_flags &= ~FS_ESCAPED;
  } else if ((flash_state.fs_flags & FS_RAW) == 0) {
    if (cc == ESCAPE_CHAR) {	/* escape character */
      flash_state.fs_flags |= FS_ESCAPED;
      return;
    }
  }
  flash_state.fs_value |= cc << flash_state.fs_shift;
  if (flash_state.fs_shift == 0)
    flash_state.fs_shift = 8;
  else {
    nvm_write(flash_state.fs_value);
    flash_state.fs_value = flash_state.fs_shift = 0;
  }
}

static void
flash_erase(uint32_t start, uint32_t end, byte flags)
{
  uint32_t addr = start;

  while (addr < end) {
    NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
    /* Set address and command */
    NVMCTRL->ADDR.reg = addr / 2;
    NVMCTRL->CTRLA.reg =
      NVMCTRL_CTRLA_CMD_ER | NVMCTRL_CTRLA_CMDEX_KEY;
    while (!(NVMCTRL->INTFLAG.bit.READY))
      ;
    addr += NVMCTRL_ROW_SIZE;
  }

  flash_state.fs_start = start;
  flash_state.fs_end = end;
  flash_state.fs_offset = start;
  flash_state.fs_shift = 0;
  flash_state.fs_flags = flags;
  flash_state.fs_value = 0;
}

static void
flash_flush()
{
  /* flush pending byte */
  if (flash_state.fs_shift)
    flash_write(0xff);

  while ((flash_state.fs_offset % NVMCTRL_ROW_SIZE) != 0) {
    nvm_write(0xffff);
  }
  while (!(NVMCTRL->INTFLAG.bit.READY))
    ;
}

/*
 * BTPIN is 14bit.
 *
 * Valid BTPIN is 0x0000 .. 0x270f
 */
#define BTPIN_VALID(x)	((x) >= 0 && (x) <= 0x270f) // 0..9999
#define BTPIN_PROBE 0x3ffd
#define BTPIN_NULL 0x3ffe

/*
 * When btpin area is erased, following value will be read.
 */
#define BTPIN_ERASED 0xffff

static void
btpin_erase()
{

  flash_erase((uint32_t)&__btpin_data_start__, (uint32_t)&__btpin_data_end__,
	      FS_RAW);
}

static void
btpin_set(uint16_t btpin)
{

  btpin_erase();

  flash_write(btpin & 0xff);
  flash_write((btpin >> 8) & 0xff);

  flash_flush();
}

static uint16_t
btpin_read()
{
  uint8_t *p = (uint8_t *)&__btpin_data_start__;

  return p[0] | (p[1] << 8);
}

static byte
btpin_write(uint16_t btpin)
{

  if (!BTPIN_VALID(btpin))
    return 0x01;
  if (btpin_read() != BTPIN_ERASED)
    return 0x02;
  btpin_set(btpin);
  return 0x00;
}

/*
 * Handler for 0x0e.  argc doesn't count 0x0e.
 */
static void
koov_sysex(byte argc, byte *argv)
{
  if (argc == 0)
    return;
  switch (argv[0]) {
  case 0x01:		// accel.
    /*
     * Request:			(argc == 2)
     *    0e 01 AA
     *          AA: 01 -> X, 02 -> Y, 03 -> Z
     * Response:
     *    0e 01 AA BB CC
     *          BB: V & 0x7f
     *          CC: (V >> 7) & 0x7f
     *          V = BB | (((signed char)(CC << 1)) << 6)
     */
    if (argc > 1) {
      int v = 0, x, y, z;
      ACCEL_UPDATE(&x, &y, &z);
      switch (argv[1]) {
      case 0x01:
	v = x;
	break;
      case 0x02:
	v = y;
	break;
      case 0x03:
	v = z;
	break;
      }
      Firmata.write(START_SYSEX);
      Firmata.write(0x0e);
      Firmata.write(0x01);
      Firmata.write(argv[1]);
      Firmata.write(v & 0x7f);
      Firmata.write((v >> 7) & 0x7f);
      Firmata.write(END_SYSEX);
    }
    break;
  case 0x02:		// Generic KOOV control
    if (argc > 1) {
      switch (argv[1]) {
      case 0x01:		/* Reset with AT commands */
	if (argc > 1) {
	  /*
	   * Request:
	   * offset 0  1  2  3  4 ...
	   * ------------------------
	   *    0e 02 01 AA BB CC ...
	   *
	   *    timeout: (AA << 7) + BB
	   *    command: CC ..
	   *
	   * Response:
	   *    0e 02 01
	   */

	  unsigned int timeout = 0;
	  if (argc > 3)
	    timeout = (argv[2] << 7) + argv[3];
	  const bool ble_connected =
	    dualStream.connectMode() == DualStream::BLE_CONNECTED;

	  Firmata.write(START_SYSEX);
	  Firmata.write(0x0e);
	  Firmata.write(0x02);
	  Firmata.write(0x01);
	  Firmata.write(END_SYSEX);

	  bts01_reset();
	  /*
	   * This small delay is neccesary so that following commands
	   * works
	   */
	  delay(500);
	  String cmd;
	  for (int i = 4; i < argc; i++) {
	    char c = argv[i];
	    if (c == '\n')
	      continue;
	    cmd.concat(c);
	    if (c == '\r') {
	      bts01_cmd(cmd.c_str(), timeout);
	      cmd = "";
	    }
	  }
	  if (ble_connected)
	    bts01_sbo();
	}
	break;
      case 0x02:		/* Exec generic AT command */
	if (argc > 3) {
	  /*
	   * Request:
	   * offset 0  1  2  3  4 ...
	   * ------------------------
	   *    0e 02 02 AA BB CC ...
	   *
	   *    timeout: (AA << 7) + BB
	   *    command: CC ..
	   *
	   * Response:
	   *    0e 02 03 reply string
	   */
	  unsigned int timeout = (argv[2] << 7) + argv[3];
	  String cmd;
	  for (int i = 4; i < argc; i++) {
	    char c = argv[i];
	    cmd.concat(c);
	  }
	  Firmata.write(START_SYSEX);
	  Firmata.write(0x0e);
	  Firmata.write(0x02);
	  Firmata.write(0x02);
	  bts01_cmd(cmd.c_str(), timeout, bts01_write, 0);
	  Firmata.write(END_SYSEX);
	}
	break;
      case 0x03:		/* Reset Firmata */
	if (argc > 2) {
	  /*
	   * Request:
	   * offset 0  1  2  3
	   * ------------------------
	   *    0e 02 03 AA BB
	   *
	   *    ticks: (AA << 7) + BB
	   */
	  unsigned int ticks = (argv[2] << 7) + argv[3];
	  initiateReset(ticks);
	}
	break;
      case 0x04:		/* buzzer */
	if (argc > 4) {
	  /*
	   * Request:
	   * offset 0  1  2  3  4
	   * ------------------------
	   *    0e 02 04 AA BB CC
	   *
	   *    pin: AA
	   *    mode: BB
	   *    freq: CC
	   *
	   * Response:
	   */
	  int pin = argv[2];
	  int mode = argv[3];
	  int freq = argv[4];

	  if (IS_PIN_DIGITAL(pin)) {
	    BUZZER_CONTROL(PIN_TO_DIGITAL(pin), mode ? HIGH : LOW, freq);
	  }
	}
	break;
      case 0x05:		/* servomotor-synchronized-motion */
	if (argc > 5) {
	  /*
	   * Request:
	   * offset 0  1  2  3  4  5  6
	   * ------------------------
	   *    0e 02 05 AA BB CC DD ...
	   *
	   *    time: AA
	   *    number: BB
	   *    pin: CC
	   *    degree: DD (0..180)
	   *
	   * Response:
	   */
	  byte time = argv[2];
	  int number = argv[3];
	  struct servo_sync ss[8];

	  if (number > sizeof(ss) / sizeof(ss[0]))
	    number = sizeof(ss) / sizeof(ss[0]);
	  if (number < ((argc - 4) / 2))
	    number = ((argc - 4) / 2);

	  struct servo_sync *p = &ss[0];
	  for (int i = 0; i < number; i++) {
	    int pin = argv[4 + i * 2];

	    if (IS_PIN_DIGITAL(pin) && pinConfig[pin] == PIN_MODE_SERVO) {
	      p->port = PIN_TO_DIGITAL(pin);
	      p->degree = argv[5 + i * 2];
	      p++;
	    }
	  }

	  SERVOMOTOR_SYNCHRONIZED_MOTION(ss, p - ss, time);
	}
	break;
      case 0x06:		/* melody */
	if (argc > 4) {
	  /*
	   * Request:
	   * offset 0  1  2  3  4
	   * ------------------------
	   *    0e 02 06 AA BB CC ...
	   *
	   *    pin: AA
	   *    tone: v = ((BB & 0x7e) >> 1)
	   *        if v == 0, buzzer-off
	   *        if v != 0, freq = v + 47
	   *    secs: (((BB & 0x01) << 8) | CC) * 10
	   *
	   * Response:
	   */
	  byte pin = argv[2];

	  if (IS_PIN_DIGITAL(pin)) {
	    for (int i = 3; i < argc - 1; i += 2) {
	      byte tone = (argv[i] & 0x7e) >> 1;
	      byte freq = tone ? tone + 47 : 0;
	      int ms = ((((argv[i] & 0x01) ? 0x100 : 0) | argv[i + 1]) * 10);

	      BUZZER_CONTROL(PIN_TO_DIGITAL(pin), tone ? HIGH : LOW, freq);
	      delay(ms);
	    }
	  }
	}
	break;
      case 0x07:		/* multi-led */
	if (argc > 4) {
	  /*
	   * Request:
	   * offset 0  1  2  3  4
	   * ------------------------
	   *    0e 02 07 AA BB CC
	   *
	   *    R: AA		// 0 .. 100
	   *    G: BB		// 0 .. 100
	   *    B: CC		// 0 .. 100
	   *
	   * Response:
	   */
	  byte r = argv[2];
	  byte g = argv[3];
	  byte b = argv[4];
	  MULTILED(r, g, b);
	}
	break;
      case 0x08:		/* flash erase */
	if (argc > 0) {
	  /*
	   * Request:
	   * offset 0  1
	   * ------------------------
	   *    0e 02 08
	   *
	   * Response:
	   * offset 0  1  2
	   * ------------------------
	   *    0e 02 08 AA
	   *
	   *    status: AA		// 0 on success
	   */
	  flash_erase((uint32_t)&__koov_data_start__, NVMCTRL_FLASH_SIZE, 0);

	  Firmata.write(START_SYSEX); /* 0xf0 */
	  Firmata.write(0x0e);
	  Firmata.write(0x02);
	  Firmata.write(0x08);
	  Firmata.write(0x00);
	  Firmata.write(END_SYSEX); /* 0xf7 */
	}
	break;
      case 0x09:		/* flash finish */
	if (argc > 0) {
	  /*
	   * Request:
	   * offset 0  1
	   * ------------------------
	   *    0e 02 09
	   *
	   * Response:
	   * offset 0  1  2
	   * ------------------------
	   *    0e 02 09 AA
	   *
	   *    status: AA		// 0 on success
	   */

	  /* The magic number should not contain nul byte */
#define KOOV_MAGIC 0x564f4f4b
	  flash_write(KOOV_MAGIC & 0xff);
	  flash_write((KOOV_MAGIC >> 8) & 0xff);
	  flash_write((KOOV_MAGIC >> 16) & 0xff);
	  flash_write((KOOV_MAGIC >> 24) & 0xff);

	  flash_flush();

	  Firmata.write(START_SYSEX); /* 0xf0 */
	  Firmata.write(0x0e);
	  Firmata.write(0x02);
	  Firmata.write(0x09);
	  Firmata.write(0x00);
	  Firmata.write(END_SYSEX); /* 0xf7 */

	  leave_firmata();
	  interp_error = 0;
	}
	break;
      case 0x0a:		/* write */
	if (argc > 1) {
	  /*
	   * Request:
	   * offset 0  1  2
	   * ------------------------
	   *    0e 02 0a AA BB ...
	   *
	   *    AA: length
	   *
	   * Response:
	   * offset 0  1  2
	   * ------------------------
	   *    0e 02 0a AA
	   *
	   *    status: AA		// 0 on success
	   */
	  byte length = argv[2];
	  int i = 3;
	  while (i < 3 + length) {
	    byte cc = argv[i++];
	    flash_write(cc);
	  }
	  Firmata.write(START_SYSEX); /* 0xf0 */
	  Firmata.write(0x0e);
	  Firmata.write(0x02);
	  Firmata.write(0x0a);
	  Firmata.write(0x00);
	  Firmata.write(END_SYSEX); /* 0xf7 */
	}
	break;
      case 0x0b:		/* btpin */
	if (argc > 1) {
	  /*
	   * Request:
	   * offset 0  1  2
	   * ------------------------
	   *    0e 02 0b AA
	   *
	   *    AA: sub command
	   *
	   * AA == 0 (write)
	   * offset 0  1  2  3  4
	   * ------------------------
	   *    0e 02 0b AA BB CC
	   *    BB: lower 7-bits of btpin
	   *    CC: higher 7-bits of btpin
	   *
	   * AA == 1 (verify)
	   * offset 0  1  2  3  4
	   * ------------------------
	   *    0e 02 0b AA BB CC
	   *    BB: lower 7-bits of btpin
	   *    CC: higher 7-bits of btpin
	   *
	   * AA == 2 (exists?)
	   * offset 0  1  2
	   * ------------------------
	   *    0e 02 0b AA
	   *
	   * Response:
	   * offset 0  1  2  3
	   * ------------------------
	   *    0e 02 0b AA BB
	   *
	   *    status: AA
	   *    status: BB		// 0 on success (except 0x02)
	   */
	  byte rv = 0x7f;

	  switch (argv[2]) {
	  case 0x00:
	    if (argc > 3) {
	      uint16_t btpin = (argv[3] & 0x7f) | ((argv[4] & 0x7f) << 7);

	      rv = btpin_write(btpin);
	    } else
	      rv = 0x7e;
	    break;
	  case 0x01:
	    if (argc > 3) {
	      uint16_t btpin = (argv[3] & 0x7f) | ((argv[4] & 0x7f) << 7);

	      rv = btpin_read() == btpin;
	    } else
	      rv = 0x7e;
	    break;
	  case 0x02:
	    rv = BTPIN_VALID(btpin_read());
	    break;
	  case 0x03:		/* challenge */
	    {
	      uint16_t mypin = btpin_read();
	      
	      if (!BTPIN_VALID(mypin)) {
		/* No valid btpin */
		rv = 0;		/* OK */
	      } else {
		if (argc > 3) {
		  uint16_t btpin = (argv[3] & 0x7f) | ((argv[4] & 0x7f) << 7);

		  if (BTPIN_VALID(btpin)) {
		    if (btpin == mypin)
		      rv = 0;	/* OK */
		    else
		      rv = 2;	/* Mismatch */
		  } else {
		    switch (btpin) {
		    default:
		    case BTPIN_PROBE:
		      rv = 2;	/* Mismatch */
		      break;
		    case BTPIN_NULL:
		      switch (dualStream.connectMode()) {
		      default:
		      case DualStream::BLE_CONNECTED:
			rv = 2;	/* Mismatch */
			break;
		      case DualStream::USB_CONNECTED:
			rv = 0;	/* OK */
			break;
		      }
		      break;
		    }
		  }
		} else {
		  rv = 2;	/* No pin given. */
		}
	      }
	    }
	    break;
	  }

	  Firmata.write(START_SYSEX); /* 0xf0 */
	  Firmata.write(0x0e);
	  Firmata.write(0x02);
	  Firmata.write(0x0b);
	  Firmata.write(argv[2]);
	  Firmata.write(rv);
	  if (argv[2] == 0x03 && rv == 0) { /* challenge */
	    Firmata.printFirmwareVersionRaw();
	  }
	  Firmata.write(END_SYSEX); /* 0xf7 */
	}
	break;
      }
    }
    break;
  }
}

void setup()
{
  /*
   * Resetting BTS01.
   */
  bts01_reset();

  // to use a port other than Serial, such as Serial1 on an Arduino Leonardo or Mega,
  // Call begin(baud) on the alternate serial port and pass it to Firmata to begin like this:
  //Serial5.begin(115200);
#if defined(USE_BLE)
  Serial.begin(38400);
  Serial.setTimeout(50);
#endif
  SerialUSB.begin(57600);

/*
 * Put BTS01 into operation mode.
 */
#if defined(USE_BLE)
#if 1
  while (!Serial)
    ;
#endif
  delay(10); // following AT commands does not complete without this

  bts01_rvn();

  if (0)
    bts01_dbi();
  bts01_ccp();
  bts01_sbo();
#endif

  leave_firmata();
  KoovSetup();
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
#define PIN_USB 20
#define PIN_BLE 21
#define PIN_AUTO 1
#define PIN_LIVE 0
#define LED_OFF(pin) do {			\
  pinMode((pin), OUTPUT);			\
  digitalWrite((pin), HIGH);			\
} while (0)
#define LED_ON(pin) do {			\
  pinMode((pin), OUTPUT);			\
  digitalWrite((pin), LOW);			\
} while (0)


static void showConnectMode();
static unsigned int blink_timer = 0;
static unsigned int blink_state = 0;

static void
blink_init()
{

  blink_timer = millis();
  blink_state = 0;
}

static int
blink_led(int pin, int interval)
{
  unsigned int now = millis();
  if (now - blink_timer > interval) {
    blink_timer = now;
    blink_state = !blink_state;
    if (pin) {
      if (blink_state)
	LED_ON(pin);
      else
	LED_OFF(pin);
    }
    return 1;
  }
  return 0;
#undef INTERVAL
}

static void
enter_firmata()
{

  enableFirmata = true;
  LED_OFF(PIN_AUTO);
  LED_ON(PIN_LIVE);
}

static void
leave_firmata()
{

  enableFirmata = false;
  LED_ON(PIN_AUTO);
  LED_OFF(PIN_LIVE);
  blink_init();
}

static void
periodc_jobs()
{
  if (dualStream.connectMode() == DualStream::NOT_CONNECTED &&
      bts01_failure) {
    blink_led(PIN_BLE, 100);
  } else {
    if (interp_error) {
      static byte hi, lo, pre;
      if (hi == 0 && lo == 0) {
	pre = 8;
	hi = ((interp_error >> 4) & 0xf) * 2;
	lo = (interp_error & 0xf) * 2;
	blink_init();
      }
      if (pre > 0) {
	if (blink_led(0, 250))
	  pre--;
      } else if (hi > 0) {
	LED_OFF(PIN_USB);
	if (blink_led(PIN_BLE, 250))
	  hi--;
      } else if (lo > 0) {
	LED_OFF(PIN_BLE);
	if (blink_led(PIN_USB, 250))
	  lo--;
      }
    } else {
      showConnectMode();
      if (enableFirmata) {
        /*
         * Since these LEDs are active low, we need to reset the state
         * after firmata initialized the ports (and firmata doesn't
         * know about state of these LEDs)..
         */
        LED_ON(PIN_LIVE);
        LED_OFF(PIN_AUTO);
      } else {
	LED_OFF(PIN_LIVE);
#if 0
	blink_led(PIN_AUTO, blink_state ? 950 : 50);
#else
        LED_ON(PIN_AUTO);
#endif
      }
    }
  }
}

static void
showConnectMode()
{
  switch (dualStream.connectMode()) {
  case DualStream::NOT_CONNECTED:
    LED_OFF(PIN_USB);
    LED_OFF(PIN_BLE);
    break;
  case DualStream::USB_CONNECTED:
    LED_ON(PIN_USB);
    LED_OFF(PIN_BLE);
    break;
  case DualStream::BLE_CONNECTED:
    LED_OFF(PIN_USB);
    LED_ON(PIN_BLE);
    break;
  };
}

void loop()
{
#if 0
  static int count = 0;
  if ((count++ % 500000) == 0) {
    DebugStream.write("z");
  }
#endif

  if (dualStream.available()) {
    enter_firmata();
    if (setupFirmata) {
      FirmataSetup();
      setupFirmata = false;
    }
  }

  periodc_jobs();

  if (enableFirmata) {
    FirmataLoop();
  }
#if !defined(DEBUG_USB)
  if (!enableFirmata) {
    KoovLoop();
  }
#endif

#undef PIN_USB
#undef PIN_BLE
#undef PIN_AUTO
#undef PIN_LIVE
#undef LED_ON
#undef LED_OFF
}

} // namespace firmata_base

static void
btpin_reset()
{

  firmata_base::btpin_set(BTPIN_ERASED);
}
