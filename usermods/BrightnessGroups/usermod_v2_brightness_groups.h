#pragma once

#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class UsermodBrightnessGroups : public Usermod {

  private:

    // Private class members. You can declare variables and functions only accessible to your usermod here
    bool enabled = false;
    bool initDone = false;
    unsigned long lastTime = 0;
    static const uint8_t max_groups = 4;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    uint8_t group_scale[max_groups+1];
    //String pixel_group[max_groups+1];
    uint8_t *pixel_groups = NULL;

    // string that are used multiple time (this will save some flash memory)
    static const char _name[];
    static const char _enabled[];

  public:

    // non WLED related methods, may be used for data exchange between usermods (non-inline methods should be defined out of class)

    /**
     * Enable/Disable the usermod
     */
    inline void enable(bool enable) { enabled = enable; }

    /**
     * Get usermod enabled/disabled state
     */
    inline bool isEnabled() { return enabled; }

    void process_pixel_group_str(uint8_t group, char *pixel_group_str) {
      if (!initDone) return;

      char *token = NULL;
      char delim[2] = ",";
      uint8_t pixel;

      token = strtok(pixel_group_str, delim);
      while (token != NULL)
      {
        pixel = atoi(token);
        if (pixel != 0 && pixel <= strip.getLengthPhysical())
        {
          pixel_groups[pixel-1] = group;
        }
        token = strtok(NULL, delim);
      }
    }

    String generate_formatted_pixel_group(uint8_t group)
    {
      String formatted = "";
      if (pixel_groups == NULL) return formatted;

      bool started = false;
      for (int pixel = 0; pixel < strip.getLengthPhysical(); pixel++)
      {
        if (pixel_groups[pixel] == group)
        {
          if (started) formatted += ",";
          else started = true;
          formatted += String(pixel+1, DEC);
        }
      }

      return formatted;
    }

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * readFromConfig() is called prior to setup()
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      // Group 0 is our default and always fixed at 100% brightness scaling
      group_scale[0] = 100;

      initDone = true;
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        //Serial.println("I'm alive!");
        lastTime = millis();
      }
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;
      //save these vars persistently whenever settings are saved
      String groupName = "Group 0";
      for (int group = 1; group <= max_groups; group++)
      {
        groupName[6] = group + '0';
        JsonObject groupJson = top.createNestedObject(groupName);
        groupJson["scale"] = group_scale[group];
        groupJson["pixels"] = generate_formatted_pixel_group(group);
      }
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();
      
      String groupName = "group0";
      String pixelGroup = "";
      for (int group = 1; group <= max_groups; group++)
      {
        groupName[5] = group + '0';

        configComplete &= getJsonValue(top[groupName]["scale"], group_scale[group], 100);
        if (group_scale[group] > 100) group_scale[group] = 100;

        configComplete &= getJsonValue(top[groupName]["pixels"], pixelGroup, "");
        if (pixel_groups == NULL)
        {
          pixel_groups = (byte*) malloc(strip.getLengthPhysical());
          if (!pixel_groups) { DEBUG_PRINTLN(F("!!! BrightnessGroup allocation failed. !!!")); return false; } //allocation failed   
        }
        
        process_pixel_group_str(group, strdup(pixelGroup.c_str()));
      }

      return configComplete;
    }


    /*
     * appendConfigData() is called when user enters usermod settings page
     * it may add additional metadata for certain entry fields (adding drop down is possible)
     * be careful not to add too much as oappend() buffer is limited to 3k
     */
    void appendConfigData()
    {
      for (int group = 1; group <= max_groups; group++)
      {
        oappend(SET_F("addInfo('"));
        oappend(String(FPSTR(_name)).c_str());
        oappend(SET_F(":group"));
        oappend(String(group, 10).c_str());
        oappend(SET_F(":scale")); 
        oappend(SET_F("',1,'<i>Local brightness value for each group between 0 and 255.</i>');"));
        
        oappend(SET_F("addInfo('"));
        oappend(String(FPSTR(_name)).c_str());
        oappend(SET_F(":group"));
        oappend(String(group, 10).c_str());
        oappend(SET_F(":pixels"));
        oappend(SET_F("',1,'Associated group for each physical pixel. Invalid or no group # will result in no impact on the pixel brightness.');"));
      }
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      if (!initDone || !enabled) return;

      // Apply brightness group scaling
      for (int pixel = 0; pixel < strip.getLengthPhysical(); pixel++)
      {
        // Get current color
        uint32_t c = strip.getPixelColor(pixel);
        
        // Breakdown into channels and apply individual brightness scaling
        uint8_t scale = group_scale[pixel_groups[pixel]];
        uint8_t w = W(c) * scale / 100.0;
        uint8_t r = R(c) * scale / 100.0;
        uint8_t g = G(c) * scale / 100.0;
        uint8_t b = B(c) * scale / 100.0;

        // Recreate and set the color
        strip.setPixelColor(pixel, RGBW32(r, g, b, w));
      }
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_BRIGHTNESS_GROUPS;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};

// add more strings here to reduce flash memory usage
const char UsermodBrightnessGroups::_name[]    PROGMEM = "BrightnessGroups";
const char UsermodBrightnessGroups::_enabled[] PROGMEM = "enabled";