#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_HALF_HEIGHT 16 // OLED display height, in pixels
#define BUTTON_FIELD_WIDTH 24  // The buttons are on line 1, write to fields this wide

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Print debug messages to serial?
#define debug 1

// Configure pins for output to joystick port 2
#define C64_PORT2_UP 2
#define C64_PORT2_DOWN 3
#define C64_PORT2_LEFT 4
#define C64_PORT2_RIGHT 5
#define C64_PORT2_FIRE 6

// Configure pins for output to joystick port 1
#define C64_PORT1_UP 8
#define C64_PORT1_DOWN 9
#define C64_PORT1_LEFT 10
#define C64_PORT1_RIGHT 11
#define C64_PORT1_FIRE 12

// Configure the pin that selects the joystick port
#define INPUT_PORT 7

// Configure the input joystick pin for NANO with 
// LED and resistor MUST BE REMOVED from the Nano.
// This pinout should also work with an Arduino Uno Rev3.
#define INPUT_UP A3
#define INPUT_DOWN A2
#define INPUT_LEFT A1
#define INPUT_RIGHT A0
#define INPUT_FIRE 13

// Current state of inputs
int state_port = 0;
int state_changed_port = 0;

#define NUM_STATES 5
#define NUM_DIR_STATES 4

// Current state of inputs
int all_states[] = { 0, 0, 0, 0, 0 };
// If states of inputs changed this loop iteration
int all_state_changes[] = { 0, 0, 0, 0, 0 };

int output2Pins[] = { C64_PORT2_UP, C64_PORT2_DOWN, C64_PORT2_LEFT, C64_PORT2_RIGHT, C64_PORT2_FIRE };
int output1Pins[] = { C64_PORT1_UP, C64_PORT1_DOWN, C64_PORT1_LEFT, C64_PORT1_RIGHT, C64_PORT1_FIRE };
int inputPins[] = { INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_FIRE };

String oledLabels[] = { "U", "D", "L", "R", "F" };


// If the OLED is present
int oled = 1;

void writeOledField(int line, String text, int x, int width) {
    if (oled) {
      int y = (line == 0) ? 0 : 16;
      // Clear the line
      display.fillRect(x, y, width, SCREEN_HALF_HEIGHT, SSD1306_BLACK);
      display.display();
      // Write the line
      display.setCursor(x, y);
      display.println(text);
      display.display();
      Serial.println(text);
    }
}

void writeOled(int line, String text) {
    writeOledField(line, text, 0, SCREEN_WIDTH);
}

void releaseOutput(int input) {
    pinMode(input, INPUT);
}

void pressOutput(int input) {
  pinMode(input, OUTPUT);
  digitalWrite(input, LOW);
}

void releaseAllOutputs() {
  // When not pressed, output ports need to default to input
  // so they float (or they will conflict with the c64 keyboard).
  for (int i = 0; i < NUM_STATES; i++) {
    releaseOutput(output1Pins[i]);
    releaseOutput(output2Pins[i]);
  }
}

void changePorts() {
  
  releaseAllOutputs();

  state_changed_port = 0;
  for (int i = 0; i < NUM_STATES; i++) {
    all_states[i] = 0;
    all_state_changes[i] = 0;
  }

  if (state_port) {
    writeOled(0, "Port 2");
  }
  else {
    writeOled(0, "Port 1");
  }
  
  // We've cleared the presses. Clear them on the display.
  writeOled(1, "");
}

void readInputs() {
  int portValue = digitalRead(INPUT_PORT);
  if (portValue == HIGH) {
    if (!state_port) {
      state_changed_port = 1;
      state_port = 1;
    }
  } 
  else {
    if (state_port) {
      state_changed_port = 1;
      state_port = 0;
    }
  }

  if (state_changed_port) {
      // Port chganed. Handle that right away.
      // This will clear all known inputs/outputs
      changePorts();
  }

  // Read the inputs
  for (int i = 0; i < NUM_STATES; i++) {
    int value = digitalRead(inputPins[i]);  
    if (value == LOW) {
      // Input grounded - button pressed.
      if (!all_states[i]) {
        all_state_changes[i] = 1;
        all_states[i] = 1;
      }
    }
    else {
      if (all_states[i]) {
        all_state_changes[i] = 1;
        all_states[i] = 0;
      }
    }
  }
}

void applyReleaseChanges() {
  for (int i = 0; i < NUM_STATES; i++) {
    if (all_state_changes[i]) {
      if (!all_states[i]) {
        releaseOutput(state_port ? output2Pins[i] : output1Pins[i]);
        writeOledField(1, "", BUTTON_FIELD_WIDTH * i, BUTTON_FIELD_WIDTH);
      }
    }
  }
}

void applyPressedChanges() {
  for (int i = 0; i < NUM_STATES; i++) {
    if (all_state_changes[i]) {
      if (all_states[i]) {
        pressOutput(state_port ? output2Pins[i] : output1Pins[i]);
        writeOledField(1, oledLabels[i], BUTTON_FIELD_WIDTH * i, BUTTON_FIELD_WIDTH);
      }
    }
  }
}

void changesComplete() {
  state_changed_port = 0;
  for (int i = 0; i < NUM_STATES; i++) {
    all_state_changes[i] = 0;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("C64 JoyTamer");

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    oled = 0;
    Serial.println("Could not init SSD1306");
  }
  if (oled) {
    Serial.println("Configuring SSD1306");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    writeOled(0, "Starting");
  }

  // Note I desoldered the LED and resistor from the NANO
  // so I could use PIN 13 as GPIO.
  // Setup input pins
  pinMode(INPUT_PORT, INPUT_PULLUP);
  for (int i = 0; i < NUM_STATES; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  // Setup output pins.
  // When not pressed, output ports need to default to input
  // so they float (or they will conflict with the c64 keyboard).
  releaseAllOutputs();

  // Determine the port to write to by reading the switch
  // And setup to output to that port.
  state_port = digitalRead(INPUT_PORT);
  changePorts();
}

void loop() {
  // put your main code here, to run repeatedly:
  readInputs();

  applyReleaseChanges();
  applyPressedChanges();

  changesComplete();
}
