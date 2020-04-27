#if defined(ESP32) || defined(ESP8266)
#include <ESP.h>
#endif

#include "hasp_hal.h"

#if ESP32
#include "esp_system.h"
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <rom/rtc.h> // needed to get the ResetInfo

void halRestart(void)
{
#if defined(ESP32) || defined(ESP8266)
    ESP.restart();
#else
    NVIC_SystemReset();
#endif
}

// Compatibility function for ESP8266 getRestInfo
String esp32ResetReason(uint8_t cpuid)
{
    if(cpuid > 1) {
        return F("Invalid CPU id");
    }
    RESET_REASON reason = rtc_get_reset_reason(cpuid);

    String resetReason((char *)0);
    resetReason.reserve(128);

    resetReason += F("CPU");
    resetReason += cpuid;
    resetReason += F(": ");

    switch(reason) {
        case 1:
            resetReason += F("POWERON");
            break; /**<1, Vbat power on reset*/
        case 3:
            resetReason += F("SW");
            break; /**<3, Software reset digital core*/
        case 4:
            resetReason += F("OWDT");
            break; /**<4, Legacy watch dog reset digital core*/
        case 5:
            resetReason += F("DEEPSLEEP");
            break; /**<5, Deep Sleep reset digital core*/
        case 6:
            resetReason += F("SDIO");
            break; /**<6, Reset by SLC module, reset digital core*/
        case 7:
            resetReason += F("TG0WDT_SYS");
            break; /**<7, Timer Group0 Watch dog reset digital core*/
        case 8:
            resetReason += F("TG1WDT_SYS");
            break; /**<8, Timer Group1 Watch dog reset digital core*/
        case 9:
            resetReason += F("RTCWDT_SYS");
            break; /**<9, RTC Watch dog Reset digital core*/
        case 10:
            resetReason += F("INTRUSION");
            break; /**<10, Instrusion tested to reset CPU*/
        case 11:
            resetReason += F("TGWDT_CPU");
            break; /**<11, Time Group reset CPU*/
        case 12:
            resetReason += F("SW_CPU");
            break; /**<12, Software reset CPU*/
        case 13:
            resetReason += F("RTCWDT_CPU");
            break; /**<13, RTC Watch dog Reset CPU*/
        case 14:
            resetReason += F("EXT_CPU");
            break; /**<14, for APP CPU, reseted by PRO CPU*/
        case 15:
            resetReason += F("RTCWDT_BROWN_OUT");
            break; /**<15, Reset when the vdd voltage is not stable*/
        case 16:
            resetReason += F("RTCWDT_RTC");
            break; /**<16, RTC Watch dog reset digital core and rtc module*/
        default:
            resetReason += F("NO_MEAN");
            return resetReason;
    }
    resetReason += F("_RESET");
    return resetReason;
}
#endif

String halGetResetInfo()
{
#if defined(ARDUINO_ARCH_ESP32)
    String resetReason((char *)0);
    resetReason.reserve(128);

    resetReason += String(esp32ResetReason(0));
    resetReason += F(" / ");
    resetReason += String(esp32ResetReason(1));
    return resetReason;
#elif defined(ARDUINO_ARCH_ESP8266)
    return ESP.getResetInfo();
#else
    return "";
#endif
}

    #ifdef __arm__
    // should use uinstd.h to define sbrk but Due causes a conflict
    extern "C" char* sbrk(int incr);
    #else  // __ARM__
    extern char *__brkval;
    #endif  // __arm__
     
    int freeMemory() {
      char top;
    #ifdef __arm__
      return &top - reinterpret_cast<char*>(sbrk(0));
    #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
      return &top - __brkval;
    #else  // __arm__
      return __brkval ? &top - __brkval : &top - __malloc_heap_start;
    #endif  // __arm__
    }

uint8_t halGetHeapFragmentation()
{
#if defined(ARDUINO_ARCH_ESP32)
    return (int8_t)(100.00f - (float)ESP.getMaxAllocHeap() * 100.00f / (float)ESP.getFreeHeap());
#elif defined(ARDUINO_ARCH_ESP8266)
    return ESP.getHeapFragmentation();
#else
    return 255;
#endif
}

size_t halGetMaxFreeBlock()
{
#if defined(ARDUINO_ARCH_ESP32)
    return ESP.getMaxAllocHeap();
#elif defined(ARDUINO_ARCH_ESP8266)
    return ESP.getMaxFreeBlockSize();
#else
    return freeMemory();
#endif
}

size_t halGetFreeHeap(void)
{
#if defined(ARDUINO_ARCH_ESP32)
    return ESP.getFreeHeap();
#elif defined(ARDUINO_ARCH_ESP8266)
    return ESP.getFreeHeap();
#else
    return 1;
#endif
}

String halGetCoreVersion()
{
#if defined(ARDUINO_ARCH_ESP32)
    return String(ESP.getSdkVersion());
#elif defined(ARDUINO_ARCH_ESP8266)
    return String(ESP.getCoreVersion());
#else
    return String(STM32_CORE_VERSION_MAJOR) + "." + STM32_CORE_VERSION_MINOR + "." + STM32_CORE_VERSION_PATCH;
#endif
}

String halGetChipModel()
{
    String model((char *)0);
    model.reserve(128);
    model = F("STM32");

#if ESP8266
    model = F("ESP8266");
#endif

#if ESP32
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    model = chip_info.cores;
    model += F("core ");
    switch(chip_info.model) {
        case CHIP_ESP32:
            model += F("ESP32");
            break;
#ifdef CHIP_ESP32S2
        case CHIP_ESP32S2:
            model += F("ESP32-S2");
            break;
 #endif
       default:
            model = F("Unknown ESP");
    }
    model += F(" rev");
    model += chip_info.revision;
#endif // ESP32

    return model;
}