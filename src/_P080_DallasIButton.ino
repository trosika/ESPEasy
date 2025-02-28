#include "_Plugin_Helper.h"
#ifdef USES_P080

// #######################################################################################################
// #################################### Plugin 080: iButton Sensor  DS1990A    ###########################
// #######################################################################################################

// Maxim Integrated

# include "src/Helpers/Dallas1WireHelper.h"

# define PLUGIN_080
# define PLUGIN_ID_080         80
# define PLUGIN_NAME_080       "Input - iButton"
# define PLUGIN_VALUENAME1_080 "iButton"


boolean Plugin_080(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_080;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_ULONG;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_080);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_080));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_bidirectional(F("1-Wire"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNote(F("External pull up resistor is needed, see docs!"));

      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      const int8_t Plugin_080_DallasPin = CONFIG_PIN1;

      if (validGpio(Plugin_080_DallasPin)) {
        Dallas_addr_selector_webform_load(event->TaskIndex, Plugin_080_DallasPin, Plugin_080_DallasPin);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // save the address for selected device and store into extra tasksettings
      Dallas_addr_selector_webform_save(event->TaskIndex, CONFIG_PIN1, CONFIG_PIN1);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      uint8_t addr[8];
      Dallas_plugin_get_addr(addr, event->TaskIndex);
      string  = Dallas_format_address(addr);
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      const int8_t Plugin_080_DallasPin = CONFIG_PIN1;

      if (validGpio(Plugin_080_DallasPin)) {
        uint8_t addr[8];

        // Explicitly set the pinMode using the "slow" pinMode function
        // This way we know for sure the state of any pull-up or -down resistor is known.
        pinMode(Plugin_080_DallasPin, INPUT);

        Dallas_plugin_get_addr(addr, event->TaskIndex);
        Dallas_startConversion(addr, Plugin_080_DallasPin, Plugin_080_DallasPin);

        delay(800); // give it time to do intial conversion
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND: // PLUGIN_READ:
    {
      uint8_t addr[8];
      Dallas_plugin_get_addr(addr, event->TaskIndex);

      if (addr[0] != 0) {
        const int8_t Plugin_080_DallasPin = CONFIG_PIN1;

        if (Dallas_readiButton(addr, Plugin_080_DallasPin, Plugin_080_DallasPin))
        {
          UserVar.setUint32(event->TaskIndex, 0, 1);
          success = true;
        }
        else
        {
          UserVar.setUint32(event->TaskIndex, 0, 0);
        }
        Dallas_startConversion(addr, Plugin_080_DallasPin, Plugin_080_DallasPin);

        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("DS   : iButton: ");

          if (success) {
            log += formatUserVarNoCheck(event->TaskIndex, 0);
          } else {
            log += F("Not Present!");
          }
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifndef BUILD_NO_DEBUG
      }
      break;
    }
    case PLUGIN_READ:
    {
      success = UserVar.getUint32(event->TaskIndex, 0) != UserVar.getUint32(event->TaskIndex, 2); // Changed?

      // Keep previous state
      UserVar.setUint32(event->TaskIndex, 2, UserVar.getUint32(event->TaskIndex, 0));
      break;
    }
  }
  return success;
}

#endif // USES_P080
