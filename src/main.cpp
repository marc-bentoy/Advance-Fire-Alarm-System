/**
 * Advance Fire Alarm System Using Arduino Uno
 * v1.0.0
 * developed by: marcb
*/

#include <Arduino.h>
#include <SoftwareSerial.h>

/*
* PIN CONFIGURATIONS
*/
// DIGITAL GPIO
#define LED_PIN     7
#define BELL_RELAY_PIN   6       // named as BELL_RELAY_PIN but **buzzer** is used during the development
#define ALARM_BUTTON_PIN 5
#define GSM_RX 8
#define GSM_TX 9
// ANALOG GPIO
#define FLAME_PIN   A5
#define SMOKE_PIN   A4

/**
 * TIME VARIABLES
*/
unsigned long start_time = 0; // used for tracking the start time of operation
unsigned long alarm_time_start = 0; // time since the alarm started 
unsigned long call_time_start = 0; // time since the call started 
unsigned long serial_print_time_start = 0; // time since the call started 
unsigned long curr_time = 0;  

/**
 * CONSTANTS
*/
const int BAUD_RATE = 9600;
const int ALARM_DURATION = 3000;
const int CALL_DURATION = 3000;
const int SMOKE_THRESHOLD = 900;
const int FIRE_THRESHOLD = 900;
const int SERIAL_WRITE_INTERVAL = 2000;

/**
 * INPUT SENSORS DATA
*/
int flame_sensor_value = 0;
int smoke_sensor_value = 0;
int alarm_button_value = 0;

/**
 * BOOLEANS
*/
bool isAlarming = false;
bool hasSentSMS = false;
bool isCalling = false;
bool shouldAlarm = false;

/**
 * CELLPHONE NUMBERS 
*/
// String BRGY_NUM = "+63239320152"; // authorized Barangay Official number
// String BFP_NUM = "+63239320152"; // Beauru of Fire Protection number
String DEV_NUM = "+63239320152"; // Project Developer number

/**
 * OBJECTS
*/
SoftwareSerial gsm(GSM_RX, GSM_TX); // GSM Serial object used for texting and calling 

void readInputs() {
  // read flame sensor data
  // flame_sensor_value = analogRead(FLAME_PIN);
  flame_sensor_value = FIRE_THRESHOLD + 100; // for development/testing

  // read smoke sensor data
  // smoke_sensor_value = analogRead(SMOKE_PIN);
  smoke_sensor_value = SMOKE_THRESHOLD + 100; // for development/testing

  // read fire alarm button
  alarm_button_value = digitalRead(ALARM_BUTTON_PIN);
  // alarm_button_value = LOW; // for development/testing
}

void alarm() {
  // turns on LED
  digitalWrite(LED_PIN, HIGH); 
  // turns on bell relay 
  digitalWrite(BELL_RELAY_PIN, HIGH);  

  // sets alarm start time as start_time to record latest alarm time
  alarm_time_start = start_time;

  isAlarming = true;
}

void stopAlarm() {
  // turns off LED
  digitalWrite(LED_PIN, LOW); 
  // turns off bell relay 
  digitalWrite(BELL_RELAY_PIN, LOW);  

  isAlarming = false;
}

void sendSMS(String message) {
  // initializes gsm command for texting 
  gsm.println("AT+CMGF=1");
  delay(100); // delay to process gsm command properly

  // writing gsm command to send recipient number
  gsm.println("AT+CMGS=\"" + DEV_NUM + "\"\r");
  delay(100); // delay to process gsm command properly
  
  // gsm command for texting 
  gsm.println(message);
  delay(100); // delay to process gsm command properly

  // gsm command for ending the text command
  gsm.println((char)26);
  delay(100); // delay to process gsm command properly

  hasSentSMS = true;

  Serial.println("---[Message Sent to: " + DEV_NUM + "]---");
}

void call() {
  // initializes gsm command for calling 
  gsm.println("AT");
  delay(500); // delay to process gsm command properly

  // gsm command for calling 
  gsm.println("ATD" + DEV_NUM + ";");
  gsm.println();

  isCalling = true;

  // writes the action to serial monitor
  Serial.println("---[Call Alarm Started to: " + DEV_NUM + "]---");
}

void stopCall() {
  // gsm command for ending the call 
  gsm.print("ATH");
  delay(100); // delay to process gsm command properly

  isCalling = false;

  // writes the action to serial monitor
  Serial.println("---[Call Alarm Ended]---");
}

void writeSerialData() {
  // return if the previous serial print does not exceed serial write interval
  if (start_time - serial_print_time_start < SERIAL_WRITE_INTERVAL) return;

  serial_print_time_start = start_time; // sets serial print time as start time to record latest serial printing 

  // starting line for serial writing
  Serial.println("----------------");

  // serial writing flame readings 
  Serial.print("🔥 Flame: ");
  Serial.println(flame_sensor_value);

  // serial writing fire alarm button readings 
  Serial.print("🔽 BUTTON: ");
  Serial.println(alarm_button_value);

  // serial writing smoke readings 
  Serial.print("💨 Smoke: ");
  Serial.println(smoke_sensor_value);

  // starting line for time variabels serial writing
  Serial.println("----------------");

  // serial writing start time 
  Serial.println("⌚ start_time: " + String(start_time));

  // serial writing last alarmed time 
  Serial.println("⏰ latest alarm time: " + String(alarm_time_start));

  // breaking line for serial writing
  Serial.println("----------------");
}

void setup() {
  // sets baud rate for Serial
  Serial.begin(BAUD_RATE);
  // sets baud rate for GSM Serial object
  gsm.begin(BAUD_RATE);

  // sets LED pin as output
  pinMode(LED_PIN, OUTPUT);
  // sets bell relay pin as output
  pinMode(BELL_RELAY_PIN, OUTPUT);
  // sets button pin as input 
  pinMode(ALARM_BUTTON_PIN, INPUT);
  // sets flame pin as input
  pinMode(FLAME_PIN, INPUT);
  // sets smoke pin as input
  pinMode(SMOKE_PIN, INPUT);

  // sets the start_time as the time from starting the machine
  start_time = millis();
}

void loop() {
  // sets start time as millis() to update starting time 
  start_time = millis();

  // reading all the input sensors 
  readInputs();

  // prints system data to serial with an interval of 2 seconds
  writeSerialData();

  // if should alarm and is not alarming
  if (shouldAlarm && !isAlarming) {
    alarm();
  }

  // if alarm button has pressed
  if (alarm_button_value == HIGH) {
    String alarmButtonPressedMessage = "❗❗WARNING❗❗\n🔽🔽🔽\nALARM BUTTON PRESSED🔽";
    Serial.println(alarmButtonPressedMessage);
    sendSMS(alarmButtonPressedMessage);
    shouldAlarm = true;
  }

  // if flame threshold has reached
  if (flame_sensor_value <= FIRE_THRESHOLD) {
    String fireThresholdReachedMessage = "❗❗WARNING❗❗\n🔥🔥🔥\nFIRE THRESHOLD REACHED🔥";
    Serial.println(fireThresholdReachedMessage);
    sendSMS(fireThresholdReachedMessage);
    shouldAlarm = true;
  }

  // if smoke threshold has reached
  if (smoke_sensor_value <= SMOKE_THRESHOLD) {
    String smokeThresholdReachedMessage = "❗❗WARNING❗❗\n💨💨💨\nSMOKE THRESHOLD REACHED💨";
    Serial.println(smokeThresholdReachedMessage);
    sendSMS(smokeThresholdReachedMessage);
    shouldAlarm = true;
  }

  // if alarm length exceeded the alarm duration 
  if (start_time - alarm_time_start > ALARM_DURATION && isAlarming) {
    shouldAlarm = false;
    stopAlarm();
    stopCall();
    isAlarming = false;
  }
}