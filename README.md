# ESP8266 HomeKit Garage Door Controller

This project turns a low-cost ESP8266 module (ESP-01) and relay board into a HomeKit-compatible garage door opener, allowing you to control your garage door using Siri, the Home app, or any HomeKit-compatible application.

## Hardware Requirements

- **ESP8266 ESP-01 Module** - The brain of the operation
- **XC3804 WiFi Relay Module** (or compatible) - Contains both an ESP-01 socket and relay
- **XC4464 USB Serial Adapter** (or similar) - For programming the ESP-01
- **5V Power Supply** - To power the module during normal operation

## Features

- **HomeKit Integration**: No additional bridges or hubs required
- **Simple Operation**: Mimics a momentary button press to activate your garage door opener
- **Reset Function**: Short TX/RX pins for 5 seconds to reset HomeKit pairing
- **Secure**: Uses native HomeKit encryption and authentication
- **Low Power**: Uses minimal electricity when idle

## How It Works

The system uses the ESP8266 to create a WiFi connection to your home network and implements the HomeKit protocol. When triggered via HomeKit, it sends a special command sequence to the relay board's microcontroller (STC15F104W), which activates the relay for a brief moment - just like pressing the button on your garage door remote.

The system presents itself to HomeKit as a simple lock mechanism, which makes it compatible with standard HomeKit commands like "Hey Siri, open the garage door".

## Installation

### 1. Hardware Setup

1. Connect the ESP-01 module to the XC3804 relay board or equivalent
2. For programming, connect the XC4464 USB adapter to the ESP-01
3. After programming, insert the ESP-01 back into the relay board
4. Connect the relay's COM and NO pins to your garage door opener's button terminals

### 2. Software Setup

1. Install the Arduino IDE (1.8.x or newer)
2. Add ESP8266 support to Arduino IDE:
   - Go to Preferences → Additional Board Manager URLs
   - Add: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Go to Tools → Board → Board Manager
   - Install "ESP8266" package

3. Install the HomeKit library:
   - Go to Sketch → Include Library → Manage Libraries
   - Search for "arduino-homekit-esp8266" and install it

4. Open `garagedoorhomekit-open-v3-withreset.ino`
5. Edit WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

6. Customize HomeKit settings in `my_accessory.c` if desired:
   - Change the default PIN code from "123-45-678"
   - Modify accessory name from "Door" if desired

7. Select correct board settings:
   - Board: "Generic ESP8266 Module"
   - Flash Size: "1MB (FS:64KB OTA:~470KB)"
   - Upload Speed: "115200"

8. Upload the code to your ESP-01

### 3. HomeKit Pairing

1. Open the Home app on your iOS device
2. Tap "+" to add a new accessory
3. Select "I Don't Have a Code or Cannot Scan"
4. Your garage door controller should appear as "Door"
5. Enter the pairing code: 123-45-678 (or your custom code)

## Usage

- In the Home app, the garage door appears as a lock
- Tap to unlock/open the garage door
- The state will automatically reset to "locked" after activation
- Use Siri commands like "Open the garage door" or "Close the garage door"

## Troubleshooting

### Factory Reset
If you need to reset the HomeKit pairing:
1. Short the TX and RX pins together for at least 5 seconds
2. The relay will click 3 times to confirm reset
3. The device will restart and be ready for new pairing

### Connection Issues
- Ensure the ESP-01 is within range of your WiFi network
- Check your WiFi credentials in the code
- Verify 5V power supply is providing adequate power

### Relay Not Triggering
- Check the connection between the ESP-01 and the relay board
- Verify the relay clicks when testing
- Check wiring between relay and garage door opener

## Technical Details

The system consists of two main components:

1. **ESP8266 (ESP-01)** running Arduino code with HomeKit implementation
2. **STC15F104W** microcontroller on the relay board that controls the physical relay

Communication between these components happens via serial UART at 9600 baud. The ESP sends specific byte sequences to control the relay:
- Relay ON: `{0xA0, 0x01, 0x01, 0xA2}`
- Relay OFF: `{0xA0, 0x01, 0x00, 0xA1}`

The HomeKit reset function works by detecting if TX and RX pins are physically shorted together, which creates a loopback effect that the code can detect.

## License

This project is released under the MIT License. See the LICENSE file for details.

## Credits

This project uses the following libraries:
- [arduino-homekit-esp8266](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266)
- ESP8266WiFi

## Safety Warning

This project involves interfacing with a garage door opener, which is a moving mechanical system. Always ensure:
- Your garage door has functional safety sensors
- The door can be manually operated in case of power failure
- All wiring is properly insulated and secured
- The device is installed in a dry, protected location

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
