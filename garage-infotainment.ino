#include <OneWire.h> // used by temperature sensors
#include <DallasTemperature.h> // used by temperature sensors
#include <Wire.h> // used by lps pressure sensor
#include <SoftwareSerial.h> // used by display
#include <LPS_Pressure.h>

// in milliseconds
#define TEMP_REFRESH_INTERVAL       3000ul
#define PRESSURE_REFRESH_INTERVAL   5000ul
#define ANIMATION_REFRESH_INTERVAL   500ul
#define LOOP_WAIT_INTERVAL            50ul

// ice warning iteration delay in milliseconds (will not be accurate)
#define ICE_WARNING_DELAY          2000ul

// temperature sensors OneWire signal pin
#define TEMP_SENSORS_PIN 12

// temperature sensors resolution (9 to 12 inclusive)
#define TEMP_RESOLUTION 9

// temperature below this is considered freezing
#define TEMP_FREEZE_WARNING 4

// screen TX and RX pins (TX pin is needed and yet not used)
#define SCREEN_TX_PIN 11
#define SCREEN_RX_PIN 10
#define SCREEN_BAUDRATE 9600
#define SCREEN_BRIGHTNESS 140

// AltIMU sensor timeouts
#define PRESSURE_READ_TIMEOUT         200

// will turn on led on pin 13 while reading sensors
#define LOOP_DELAY_LED_PIN 13

// ice warning string
#define ICE_WARNING_STRING "risk of ice"

// temperature chip i/o
OneWire oneWire(TEMP_SENSORS_PIN);
DallasTemperature tempSensors(&oneWire);

//DS18S20 hardware addresses
DeviceAddress InTemp =  { 0x28, 0xFF, 0x7E, 0x74, 0x06, 0x00, 0x00, 0xF0 };
DeviceAddress OutTemp = { 0x28, 0x4E, 0x76, 0x74, 0x06, 0x00, 0x00, 0xFE };

// screen dim button pin
//const int ScreenDimBtnPin = 7;

// screen dim values
//const int ScreenDimOn = 140;
//const int ScreenDimOff = 150;

// AltIMU sensors
LPS pressureSensor = LPS();

// AltIMU sensors detection
boolean pressureSensorDetected = false;

// last screen symbol on the second line
char animationChars[] = "^<^>";
int loadingIndex = 0;

// arrow showed on no AltIMU response
char arrow[] = "->";

// screen output
SoftwareSerial screenSerial(SCREEN_TX_PIN, SCREEN_RX_PIN);

// async update milliseconds
unsigned long lastTemperatureMillis = millis();
unsigned long lastPressureMillis = millis();
unsigned long lastAnimationMillis = millis();

// ice warning iterations
int currentIceWarningIterations = 0;
int maxIceWarningIterations = ICE_WARNING_DELAY / LOOP_WAIT_INTERVAL;
boolean showIceWarning = false;
boolean shownIceWarning = false;

void setup(void) {
  // for debugging purposes
  // Serial.begin(9600);

  // setting up screen
  screenSerial.begin(SCREEN_BAUDRATE);
  clearScreen();
  changeScreenBrightness(SCREEN_BRIGHTNESS);

  // setting up Wire instance for pressure and magnetic
  // sensor
  Wire.begin();

  setupTemperatureSensors();
  setupAltIMUSensors();

  pinMode(LOOP_DELAY_LED_PIN, OUTPUT);
  digitalWrite(LOOP_DELAY_LED_PIN, LOW);
}

void setupTemperatureSensors() {
  // setting up temperature sensors
  tempSensors.begin();

  // setting the resolution of the temperature sensors
  tempSensors.setResolution(InTemp, TEMP_RESOLUTION);
  tempSensors.setResolution(OutTemp, TEMP_RESOLUTION);
}

void setupAltIMUSensors() {
  // setting up atmospheric pressure sensor
  pressureSensor.setTimeout(PRESSURE_READ_TIMEOUT);
  if (pressureSensor.init()) {
    pressureSensorDetected = true;
    pressureSensor.enableDefault();
  }
}

void loop(void) {
  checkIfShouldUpdateAnimation(ANIMATION_REFRESH_INTERVAL);
  
  if (!checkIfShouldUpdateTemperatureValues(TEMP_REFRESH_INTERVAL) && !showIceWarning) {
    checkIfShouldUpdatePressureValue(PRESSURE_REFRESH_INTERVAL);
  } else if (showIceWarning) {
    displayIceWarning();

    currentIceWarningIterations++;
    if (currentIceWarningIterations == maxIceWarningIterations) {
      shownIceWarning = true;
      showIceWarning = false;
      clearSecondLine();
    }
  }
  
  digitalWrite(LOOP_DELAY_LED_PIN, LOW);
  delay(LOOP_WAIT_INTERVAL);
  digitalWrite(LOOP_DELAY_LED_PIN, HIGH);
}

// all methods below will return true if their screen values
// have been updated
boolean checkIfShouldUpdateTemperatureValues(unsigned long interval) {
  unsigned long currentMillis = millis();

  if (currentMillis - lastTemperatureMillis >= interval) {
//    Serial.println("will update temperatures");
    lastTemperatureMillis = currentMillis;

    // get temperature values from sensors
    tempSensors.requestTemperatures();

    // temperature is in Celsius
    float temperatureIn = tempSensors.getTempC(InTemp);
    float temperatureOut = tempSensors.getTempC(OutTemp);

    if (!shownIceWarning && temperatureOut <= TEMP_FREEZE_WARNING) {
      showIceWarning = true;
    }

    refreshDisplayFirstLine(temperatureIn, temperatureOut);

    return true;
  }

  return false;
}

boolean checkIfShouldUpdatePressureValue(unsigned long interval) {
  unsigned long currentMillis = millis();

  if (currentMillis - lastPressureMillis >= interval) {
//    Serial.println("will update pressure");
    lastPressureMillis = currentMillis;

    float pressureInKpa = -1;
    if (pressureSensorDetected) {
      float pressureInMillibars = pressureSensor.readPressureMillibars();

      if (!pressureSensor.timeoutOccurred()) {
        pressureInKpa = pressureInMillibars * (float)0.1;
      } else {
        pressureSensorDetected = false;
      }
    } else {
      setupAltIMUSensors();
    }

    refreshDisplaySecondLinePressure(pressureInKpa);

    return true;
  }

  return false;
}

boolean checkIfShouldUpdateAnimation(unsigned long interval) {
  unsigned long currentMillis = millis();

  if (currentMillis - lastAnimationMillis >= interval) {
//    Serial.println("will update animation");
    lastAnimationMillis = currentMillis;

    updateAnimation();

    return true;
  }

  return false;
}
