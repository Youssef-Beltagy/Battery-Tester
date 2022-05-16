#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define CONTROL_PIN 23
#define START_PIN 19
#define BAUD_RATE 9600
#define MAX_NUM_COMMANDS 10

struct Command{
  String description;
  int (*func)(void);
};

int num_commands;
int cur_command;
Command commands[MAX_NUM_COMMANDS];

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int measure_1_5_bat(){
  float voltage = analogRead(36) * 3.3 /4095;

  // Thresholds are guesstimates using the duracell MN1500 datasheet and heuristic investigations
  String status = "Good Battery";
  if(voltage < 1.0) status = "Ugly Battery"; // less than 2/11 of battery remaining at very low voltage
  else if (voltage < 1.25) status = "Bad Battery"; // less than 1/2 of battery remaining

  display.printf("Voltage: %.2f\nStatus: %s\n", voltage, status);
  return 2000;
}

void display_home(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Measuring Batteries");
  display.println("L to toggle, R to run\n");
  display.println("Current Command: \n");
  display.println(commands[cur_command].description);
  display.display();
}

void setup() {

  Serial.begin(BAUD_RATE);
  pinMode(CONTROL_PIN, INPUT_PULLUP);
  pinMode(START_PIN, INPUT_PULLUP);

  cur_command = 0;
  commands[num_commands++] = {"Measure 1.5V BAT", measure_1_5_bat};

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  Serial.println(F("SSD1306 allocation succeeded"));
  delay(1000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);


  display.println("Testing Batteries\n");
  display.println("Connect a battery and choose a command. Run the command to know if the battery is good, bad, or UGLY");
  display.display();
  delay(5000);

  display_home();
  delay(1000);
 
}

void loop() {

  if(digitalRead(CONTROL_PIN) == LOW){
    Serial.println("Control pin");
    cur_command = (cur_command + 1) % num_commands;
    display_home();

    // Lazy debouncing by spin waiting
    while(digitalRead(CONTROL_PIN) == LOW) ;
    delay(50);
  }

  if(digitalRead(START_PIN) == LOW){
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Current Command: ");
    display.println(commands[cur_command].description);
    display.println();
    int delay_for = commands[cur_command].func();
    display.display();
    delay(delay_for);
    display_home();
  }

}