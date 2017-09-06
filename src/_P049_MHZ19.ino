/*

  This plug in is written by Dmitry (rel22 ___ inbox.ru)
  Plugin is based upon SenseAir plugin by Daniel Tedenljung info__AT__tedenljungconsulting.com
  Additional features based on https://geektimes.ru/post/285572/ by Gerben (infernix__AT__gmail.com)

  This plugin reads the CO2 value from MH-Z19 NDIR Sensor
  DevicePin1 - is RX for ESP
  DevicePin2 - is TX for ESP
*/

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_049
#define PLUGIN_ID_049         49
#define PLUGIN_NAME_049       "Gases - CO2 MH-Z19 [TESTING]"
#define PLUGIN_VALUENAME1_049 "PPM"
#define PLUGIN_VALUENAME2_049 "Temperature" // Temperature in C
#define PLUGIN_VALUENAME3_049 "U" // Undocumented, minimum measurement per time period?
#define PLUGIN_READ_TIMEOUT   3000

boolean Plugin_049_init = false;
// Default of the sensor is to run ABC
boolean Plugin_049_ABC_Disable = false;
boolean Plugin_049_ABC_MustApply = false;

enum
{
  ABC_enabled  = 0x01,
  ABC_disabled = 0x02
};

#include <SoftwareSerial.h>
SoftwareSerial *Plugin_049_SoftSerial;

byte mhzData[9] ={0, 0, 0, 0, 0, 0, 0, 0, 0};    // 9 bytes bytes command
byte mhzResp[9];    // 9 bytes bytes response

enum {
  mhzReadPPM = 0,
  mhzCalibrateZero,
  mhzABCEnable,
  mhzABCDisable,
  mhzReset,
  mhzMeasurementRange1000,
  mhzMeasurementRange2000,
  mhzMeasurementRange3000,
  mhzMeasurementRange5000,
  mhzUnknownCommand
};

boolean Plugin_049_parseCommand(const String& command, byte &commandEnum) {
  commandEnum = mhzUnknownCommand;
  if (command == F("mhzcalibratezero")) commandEnum = mhzCalibrateZero;
  if (command == F("mhzreset")) commandEnum = mhzReset;
  if (command == F("mhzabcenable")) commandEnum = mhzABCEnable;
  if (command == F("mhzabcdisable")) commandEnum = mhzABCDisable;
  if (command == F("mhzmeasurementrange1000")) commandEnum = mhzMeasurementRange1000;
  if (command == F("mhzmeasurementrange2000")) commandEnum = mhzMeasurementRange2000;
  if (command == F("mhzmeasurementrange3000")) commandEnum = mhzMeasurementRange3000;
  if (command == F("mhzmeasurementrange5000")) commandEnum = mhzMeasurementRange5000;
  return commandEnum != mhzUnknownCommand;
}

unsigned int Plugin_049_getMeasurementRange(byte command) {
  switch (command) {
    case mhzMeasurementRange1000: return 1000;
    case mhzMeasurementRange2000: return 2000;
    case mhzMeasurementRange3000: return 3000;
    case mhzMeasurementRange5000: return 5000;
  }
  return 0;
}

boolean Plugin_049_executeCommand(byte command) {
  String logString = F("MHZ19: ");
  boolean showLog = true;
  memset(mhzData, 0, sizeof(mhzData));
  mhzData[0] = 0xFF; // Starting Byte
  mhzData[1] = 0x01; // Sensor Number
  unsigned int byte6_7 = 0;
  switch (command)
  {
    case mhzReadPPM:
      mhzData[2] = 0x86;
      showLog = false;
      break;
    case mhzCalibrateZero:
      mhzData[2] = 0x87;
      logString += F("Calibrated zero point!");
      break;
    case mhzReset:
      mhzData[2] = 0x8d;
      logString += F("Sent sensor reset!");
      break;
    case mhzABCEnable:
      mhzData[2] = 0x79;
      mhzData[3] = 0xA0;
      logString += F("Sent sensor ABC Enable!");
      break;
    case mhzABCDisable:
      mhzData[2] = 0x79;
      mhzData[3] = 0x00;
      logString += F("Sent sensor ABC Disable!");
      break;
    case mhzMeasurementRange1000:
    case mhzMeasurementRange2000:
    case mhzMeasurementRange3000:
    case mhzMeasurementRange5000:
      mhzData[2] = 0x99;
      byte6_7 = Plugin_049_getMeasurementRange(command);
      mhzData[6] = (byte6_7 >> 8) & 0xFF;
      mhzData[7] = byte6_7 & 0xFF;
      logString += F("Sent measurement range 0-");
      logString += byte6_7;
      logString += F("PPM!");
      break;
    default:
      return false;
  }
  // Add CRC
  byte crc = 0;
  for (int i = 1; i < 8; i++) crc += mhzData[i];
  crc = 255 - crc;
  crc++;
  mhzData[8] = crc;
  // Write the command
  int nbBytesSent = Plugin_049_SoftSerial->write(mhzData, 9);
  if (nbBytesSent != 9) {
    logString = F("MHZ19: Error, nb bytes sent != 9 : ");
    logString += nbBytesSent;
    addLog(LOG_LEVEL_INFO, logString);
    return false;
  }
  if (showLog) addLog(LOG_LEVEL_INFO, logString);
  return true;
}

boolean Plugin_049(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_049;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_049);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_049));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_049));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_049));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2] = { F("Normal"), F("ABC disabled") };
        int optionValues[2] = { ABC_enabled, ABC_disabled };
        addFormSelector(string, F("Auto Base Calibration"), F("plugin_049_abcdisable"), 2, options, optionValues, choice);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        const int formValue = getFormItemInt(F("plugin_049_abcdisable"));
        boolean new_ABC_disable = (formValue == ABC_disabled);
        if (Plugin_049_ABC_Disable != new_ABC_disable) {
          // Setting changed in the webform.
          Plugin_049_ABC_MustApply = true;
          Plugin_049_ABC_Disable = new_ABC_disable;
        }
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = formValue;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_049_ABC_Disable) {
          // No guarantee the correct state is active on the sensor after reboot.
          Plugin_049_ABC_MustApply = true;
        }
        Plugin_049_ABC_Disable = Settings.TaskDevicePluginConfig[event->TaskIndex][0] == ABC_disabled;
        Plugin_049_SoftSerial = new SoftwareSerial(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]);
        Plugin_049_SoftSerial->begin(9600);
        addLog(LOG_LEVEL_INFO, F("MHZ19: Init OK "));

        //delay first read, because hardware needs to initialize on cold boot
        //otherwise we get a weird value or read error
        timerSensor[event->TaskIndex] = millis() + 15000;

        Plugin_049_init = true;
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        byte commandEnum = mhzUnknownCommand;
        if (Plugin_049_parseCommand(command, commandEnum)) {
          if (Plugin_049_executeCommand(commandEnum)) {
            success = true;
          }
        }
        break;
      }

    case PLUGIN_READ:
      {

        if (Plugin_049_init)
        {
          Plugin_049_executeCommand(mhzReadPPM);
          // clear previous content of response
          memset(mhzResp, 0, sizeof(mhzResp));
          // Start reading response
          long start = millis();
          int counter = 0;
          while (((millis() - start) < PLUGIN_READ_TIMEOUT) && (counter < 9)) {
            if (Plugin_049_SoftSerial->available() > 0) {
              mhzResp[counter++] = Plugin_049_SoftSerial->read();
            } else {
              delay(10);
            }
          }
          if (counter < 9){
              addLog(LOG_LEVEL_INFO, F("MHZ19: Error, timeout while trying to read"));
          }

          unsigned int ppm = 0;
          int i;
          signed int temp = 0;
          unsigned int s = 0;
          float u = 0;
          byte crc = 0;
          for (i = 1; i < 8; i++) crc+=mhzResp[i];
              crc = 255 - crc;
              crc++;

          if ( !(mhzResp[8] == crc) ) {
             String log = F("MHZ19: Read error : CRC = ");
             log += String(crc); log += " / "; log += String(mhzResp[8]);
             log += " bytes read  => ";for (i = 0; i < 9; i++) {log += mhzResp[i];log += "/" ;}
             addLog(LOG_LEVEL_ERROR, log);

             // Sometimes there is a misalignment in the serial read
             // and the starting byte 0xFF isn't the first read byte.
             // This goes on forever.
             // There must be a better way to handle this, but here
             // we're trying to shift it so that 0xFF is the next byte
             byte crcshift;
             for (i = 1; i < 8; i++) {
                crcshift = Plugin_049_SoftSerial->peek();
                if (crcshift == 0xFF) {
                  String log = F("MHZ19: Shifted ");
                  log += i;
                  log += F(" bytes to attempt to fix buffer alignment");
                  addLog(LOG_LEVEL_ERROR, log);
                  break;
                } else {
                  crcshift = Plugin_049_SoftSerial->read();
                }
             }
             success = false;
             break;

          // Process responses to 0x86
          } else if (mhzResp[0] == 0xFF && mhzResp[1] == 0x86 && mhzResp[8] == crc)  {

              //calculate CO2 PPM
              unsigned int mhzRespHigh = (unsigned int) mhzResp[2];
              unsigned int mhzRespLow = (unsigned int) mhzResp[3];
              ppm = (256*mhzRespHigh) + mhzRespLow;

              // set temperature (offset 40)
              unsigned int mhzRespTemp = (unsigned int) mhzResp[4];
              temp = mhzRespTemp - 40;

              // set 's' (stability) value
              unsigned int mhzRespS = (unsigned int) mhzResp[5];
              s = mhzRespS;

              // calculate 'u' value
              unsigned int mhzRespUHigh = (unsigned int) mhzResp[6];
              unsigned int mhzRespULow = (unsigned int) mhzResp[7];
              u = (256*mhzRespUHigh) + mhzRespULow;

              String log = F("MHZ19: ");

              // During (and only ever at) sensor boot, 'u' is reported as 15000
              // We log but don't process readings during that time
              if (u == 15000) {

                log += F("Bootup detected! ");
                if (Plugin_049_ABC_Disable) {
                  // After bootup of the sensor the ABC will be enabled.
                  // Thus only actively disable after bootup.
                  Plugin_049_ABC_MustApply = true;
                  log += F("Will disable ABC when bootup complete. ");
                }
                success = false;

              // If s = 0x40 the reading is stable; anything else should be ignored
              } else if (s < 64) {

                log += F("Unstable reading, ignoring! ");
                success = false;

              // Finally, stable readings are used for variables
              } else {

                UserVar[event->BaseVarIndex] = (float)ppm;
                UserVar[event->BaseVarIndex + 1] = (float)temp;
                UserVar[event->BaseVarIndex + 2] = (float)u;
                if (Plugin_049_ABC_MustApply) {
                  // Send ABC enable/disable command based on the desired state.
                  if (Plugin_049_ABC_Disable) {
                    Plugin_049_executeCommand(mhzABCDisable);
                  } else {
                    Plugin_049_executeCommand(mhzABCEnable);
                  }
                  Plugin_049_ABC_MustApply = false;
                }
                success = true;
              }

              // Log values in all cases
              log += F("PPM value: ");
              log += ppm;
              log += F(" Temp/S/U values: ");
              log += temp;
              log += F("/");
              log += s;
              log += F("/");
              log += u;
              addLog(LOG_LEVEL_INFO, log);
              break;

          // Sensor responds with 0x99 whenever we send it a measurement range adjustment
          } else if (mhzResp[0] == 0xFF && mhzResp[1] == 0x99 && mhzResp[8] == crc)  {

            addLog(LOG_LEVEL_INFO, F("MHZ19: Received measurement range acknowledgment! "));
            addLog(LOG_LEVEL_INFO, F("Expecting sensor reset..."));
            success = false;
            break;

          // log verbosely anything else that the sensor reports
          } else {

              String log = F("MHZ19: Unknown response:");
              for (int i = 0; i < 9; ++i) {
                log += F(" ");
                log += String(mhzResp[i], HEX);
              }
              addLog(LOG_LEVEL_INFO, log);
              success = false;
              break;

          }

        }
        break;
      }
  }
  return success;
}

#endif
