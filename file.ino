#include <math.h>
#include <EEPROM.h>

#define sampleWindow 50
#define SENSOR_PIN A0

#define LED1 9
#define LED2 10
#define LED3 11
#define LED4 6

#define BUTTON 2

int mode = 0;
bool lastButtonState = HIGH;
unsigned long lastButtonPressTime = 0;
const long debounceDelay = 50;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT);
  Serial.begin(115200);

  EEPROM.get(0, mode);
  if (mode < 0 || mode > 4) {
    mode = 0;  // Default to mode 0 if the value is corrupted
  }
  Serial.print("Loaded mode: ");
  Serial.println(mode);
}

void loop() {
  checkButton();

  if (mode == 0) {
    voice();
    Serial.println("voice");
  } else if (mode == 1) {
    fade();
    Serial.println("fade");
  } else if (mode == 2) {
    knightRider();
    Serial.println("knightrider");
  } else if (mode == 3) {
    breathing();
    Serial.println("breathing");
  } else {
    strobe();
    Serial.println("strobe");
  }
}

void voice() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  unsigned int sample;

  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(SENSOR_PIN);
    if (sample < 1024) {
      if (sample > signalMax) {
        signalMax = sample;
      } else if (sample < signalMin) {
        signalMin = sample;
      }
    }
  }

  float peakToPeak = signalMax - signalMin;
  float db = map(peakToPeak, 20, 900, 50, 90);
  int brightness = map(db, 50, 90, 0, 255);

  analogWrite(LED1, brightness);
  analogWrite(LED2, brightness);
  analogWrite(LED3, brightness);
  analogWrite(LED4, brightness);
}

void fade() {
  static int brightness = 0;
  static int fadeDirection = 1;
  static unsigned long lastFadeTime = 0;
  const int fadeDelay = 5;

  if (millis() - lastFadeTime > fadeDelay) {
    lastFadeTime = millis();
    brightness += fadeDirection;

    if (brightness >= 255) {
      fadeDirection = -1;
      brightness = 255;
    } else if (brightness <= 0) {
      fadeDirection = 1;
      brightness = 0;
    }

    analogWrite(LED1, brightness);
    analogWrite(LED2, brightness);
    analogWrite(LED3, brightness);
    analogWrite(LED4, brightness);
  }
}

void strobe() {
  static int ledState = LOW;  // Track the current state of the LEDs (on/off)
  static unsigned long lastStrobeTime = 0;
  const int strobeDelay = 50;  // The delay in milliseconds between flashes

  if (millis() - lastStrobeTime > strobeDelay) {
    lastStrobeTime = millis();  // Update the timer

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // Apply the new state to all LEDs
    digitalWrite(LED1, ledState);
    digitalWrite(LED2, ledState);
    digitalWrite(LED3, ledState);
    digitalWrite(LED4, ledState);
  }
}

void knightRider() {
  static int currentLed = 0;
  static int direction = 1;
  static unsigned long lastMoveTime = 0;
  const int moveDelay = 100;
  int numLeds = 4;

  // LED pins stored in an array for cleaner code
  int ledPins[] = { LED1, LED2, LED3, LED4 };

  if (millis() - lastMoveTime > moveDelay) {
    lastMoveTime = millis();

    // Turn off all LEDs first using a loop
    for (int i = 0; i < numLeds; i++) {
      analogWrite(ledPins[i], 0);
    }

    // Light up the current LED
    analogWrite(ledPins[currentLed], 255);

    // Move to the next LED
    currentLed += direction;

    // Reverse direction at the ends
    if (currentLed == numLeds || currentLed < 0) {
      direction = -direction;
      // After reversing, the next LED in the sequence will be correct
      // (e.g., from 4, direction is -1, next loop moves to 3)
      currentLed += direction;
    }
  }
}

void breathing() {
  // Use a sine wave to create a smooth, pulsating effect.
  // The '500.0' value controls the speed of the breathing effect.
  // A larger number makes the pulse slower.
  float val = (sin(millis() / 500.0) + 1.0) / 2.0 * 255.0;
  int brightness = (int)val;

  // Apply the breathing brightness to all LEDs using defined macros
  analogWrite(LED1, brightness);
  analogWrite(LED2, brightness);
  analogWrite(LED3, brightness);
  analogWrite(LED4, brightness);
}

// void checkButton() {
//   bool buttonState = digitalRead(BUTTON);

//   if (buttonState != lastButtonState) {
//     lastButtonPressTime = millis();
//   }

//   if ((millis() - lastButtonPressTime) > debounceDelay) {
//     if (buttonState == LOW && lastButtonState == HIGH) {
//       mode = (mode + 1) % 5;
//     }
//   }

//   lastButtonState = buttonState;
// }

void checkButton() {
  bool switchState = digitalRead(BUTTON);
  static bool lastSwitchState = LOW;

  if (switchState == HIGH && lastSwitchState == LOW) {
    mode = (mode + 1) % 5;

    EEPROM.put(0, mode);

    Serial.println(mode);
  }

  lastSwitchState = switchState;
}
