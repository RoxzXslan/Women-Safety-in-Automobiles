#include <LiquidCrystal_I2C.h>
#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define pushButton 2
#define buzzer 5

#define TX 10 // TX of SIM 808
#define RX 11 // RX of SIM 808
SoftwareSerial mySerial(TX, RX);
DFRobot_SIM808 sim808(&mySerial);

char phone1[] = "+916379161303";  // predefined phone number 1
char phone2[] = "+918608071697";  // predefined phone number 2

unsigned long duration = 2 * 60 * 60 * 1000; // 2 hours in milliseconds

void setup() {
  Serial.begin(9600);
  // Initializing LCD Module
  lcd.init();
  lcd.backlight();

  displayData(0, "Welcome", 4, "User", 500); // user can be replaced with their name max 12 chars

  pinMode(buzzer, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);
  digitalWrite(buzzer, 0);

  mySerial.begin(9600);

  displayData(0, "Initialising", 0, "");
  // Initializing SIM808 Module
  while(!sim808.init()) {
    delay(1000);
    Serial.print("Sim808 init error\r\n");
  }
  displayData(0, "Initialising", 4, "Successful", 500);
  // Turning ON GPS
  while(!sim808.attachGPS());
  Serial.println("GPS powered ON successfully");
  displayData(0, "GPS", 4, "Turned ON", 500);
}

void loop() {
  float lat, lon; // to store latitude and longitude coordinates
  displayData(0, "Getting", 4, "GPS Data");
  getGPSdata(lat, lon); // Getting GPS data - latitude, longitude 
  displayGPSData(String(lat, 6), String(lon, 6)); // displaying the GPS data on LCD with 6 decimal places
  char la[11], lo[11];
  dtostrf(lat, 4, 6, la); dtostrf(lon, 4, 6, lo);
  
  char message[100];
  sprintf(message, "Emergency!!! \nLatitude  : %s\nLongitude : %s\n", la,  lo); // prepare the message with the GPS data

  if(!digitalRead(pushButton))  // need to press the button for atleast 0.5 sec due to delays and functions
    Alert(message);
  
  delay(500);
}

// Function to initiate the alert and buzzer
void Alert(char* message) {
  unsigned long start_time = millis(); // taking the starting time
  digitalWrite(buzzer, 1); // turn ON the buzzer
  SendSMS(message);  // sending SMS
  digitalWrite(buzzer, 0); // turn OFF the buzzer once the message sent to make sure the sms has been sent (a small beep will be generated to show that message has been sent)
  displayData(0,"Alert",4,"Sent",1000); // displaying that sms has been sent for 1 sec

  while(millis()-start_time < duration) { // checking the current time with starting time and running the program till the required duration
    digitalWrite(buzzer, 1);  // turn ON the buzzer
    if(checkForSTOP())  break;
    getGPSdata(); // function to always show the gps coordinate of the user
    digitalWrite(buzzer, 0);  // turn off the buzzer
  } 
  digitalWrite(buzzer,0); // turning off the buzzer
  displayData(0,"Buzzer",4,"Turned OFF",500);
}

// Function to check for the STOP message
bool checkForSTOP(void) {
  int messageIndex = sim808.isSMSunread();  // checks for any unread SMS

  if(messageIndex > 0) {
    char message[160];  // stores the message received
    char phoneN[16];    // stores the phone number the message is from
    char datetime[24];  // stores the time and date at which the message is received
    sim808.readSMS(messageIndex, message, 160, phoneN, datetime);  // reads the unread SMS
    String msg = message;
    sim808.deleteSMS(messageIndex); // delete the SMS after reading
    if(msg.equalsIgnoreCase("STOP") && checkPhone(phoneN))  // if message is Stop (any case format) from pre-defined phone number, returns true
      return true;
  }
  return false;
}

// Function to check whether the phone number belongs to predefined phone number
bool checkPhone(String phoneN) {
  if(phoneN.equals(String(phone1)) || phoneN.equals(String(phone2)))
    return true;
  return false;
}

// Function to get the GPS data and store them in the input argument
void getGPSdata(float& lat, float& lon) {
  while(!sim808.getGPS());  // getting gps
  lat = sim808.GPSdata.lat; // latitude data
  lon = sim808.GPSdata.lon; // longitude data
}

// Function to get the GPS data and displays the data on lcd by itself
void getGPSdata() {
  while(!sim808.getGPS());  // getting gps
  float lat = sim808.GPSdata.lat; // latitude data
  float lon = sim808.GPSdata.lon; // longitude data

  displayGPSData(String(lat, 6), String(lon, 6)); // displaying the GPS data on LCD with 6 decimal places
}

// Function to display GPS data on a LCD with two rows r1 -> latitude, r2 -> longitude
void displayGPSData(String lat, String lon) {
  lcd.clear(); delay(1);
  lcd.setCursor(0, 0);  lcd.print("Lat : " + lat);  // row 1 -> latitude
  lcd.setCursor(0, 1);  lcd.print("Lon : " + lon);  // row 2 -> longitude
}

// Function to display data on a LCD with two rows, with a duration to display attached
void displayData(int pos1, String data1, int pos2, String data2, unsigned long dur) {
  lcd.clear(); delay(1);
  lcd.setCursor(pos1, 0);  lcd.print(data1);  // row1 -> pos1 defines the column
  lcd.setCursor(pos2, 1);  lcd.print(data2);  // row2 -> pos2 defines the column
  delay(dur); // delay for the screen to get clear
  lcd.clear();
}

// Function to display data on a LCD with two rows
void displayData(int pos1, String data1, int pos2, String data2) {
  lcd.clear(); delay(1);
  lcd.setCursor(pos1, 0);  lcd.print(data1);  // row1 -> pos1 defines the column
  lcd.setCursor(pos2, 1);  lcd.print(data2);  // row2 -> pos2 defines the column
}

// Function to send an SMS to a predefined phone number and a desired message
void SendSMS(char* Message) {
  sim808.sendSMS(phone1, Message); // sends SMS to the pre-defined phone number 1
  sim808.sendSMS(phone2, Message); // sends SMS to the pre-defined phone number 2
  // can edit these phone numbers as per necessary
}
