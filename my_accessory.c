/*
 * my_accessory.c
 * Define the accessory as a simple lock mechanism
 * This allows using "open the door" commands while keeping code simple
 */

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// Called to identify this accessory
void my_accessory_identify(homekit_value_t _value) {
  // Just trigger the relay once for identification
}

// Lock mechanism characteristics - using minimalist approach
homekit_characteristic_t cha_lock_target_state = HOMEKIT_CHARACTERISTIC_(LOCK_TARGET_STATE, 1); // 1 = secured/locked
homekit_characteristic_t cha_lock_current_state = HOMEKIT_CHARACTERISTIC_(LOCK_CURRENT_STATE, 1); // 1 = secured/locked
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "Door");

// Define the accessory
homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_door, .services=(homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Door"),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, "DIY"),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "1234567890"),
      HOMEKIT_CHARACTERISTIC(MODEL, "ESP01"),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
    HOMEKIT_SERVICE(LOCK_MECHANISM, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &cha_lock_target_state,
      &cha_lock_current_state,
      &cha_name,
      NULL
    }),
    NULL
  }),
  NULL
};

// Define the HomeKit configuration
homekit_server_config_t config = {
  .accessories = accessories,
  .password = "123-45-678"
};