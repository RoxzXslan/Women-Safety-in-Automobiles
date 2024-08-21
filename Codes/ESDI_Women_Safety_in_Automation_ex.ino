#include <LiquidCrystal_I2C.h>
#include <DFRobot_SIM808.h>
#include <string.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define pushButton 2
#define buzzer 5

#define TX 10
#define RX 11
SoftwareSerial mySerial(TX, RX);
DFRobot_SIM808 sim808(&mySerial);

char phone[16] = "+916379161303";

void getGPSdata(float& lat, float& lon) {
  while(!sim808.getGPS());
  lat = sim808.GPSdata.lat;
  lon = sim808.GPSdata.lon;
}

void displayGPSdata(String lat, String lon) {
  lcd.setCursor(0, 0);  lcd.print("Latitude : " + lat);
  lcd.setCursor(0, 1);  lcd.print("Longitude: " + lon);
}

void SendSMS(char phoneNumber, char Message) {
  sim808.sendSMS((char *) phoneNumber, (char *) Message);
}

bool checkForSTOP() {
  int messageIndex = sim808.isSMSunread();

  int Message_length = 160;
  char message[Message_length];
  char phoneN[16];
  char datetime[24];

  if(messageIndex > 0) {
    sim808.readSMS(messageIndex, message, Message_length, phoneN, datetime);
    String msg = message;
    if(msg.equalsIgnoreCase("STOP"))
      return true;
  }
  return false;
}

void Buzzer(bool Switch) {
  // program for buzzer
  digitalWrite(buzzer, Switch);
}

void setup() {
  // Initialising LCD Module
  lcd.init();

  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Welcome"); // Printing Welcome address on LCD
  delay(500);
  lcd.clear();

  Serial.begin(9600);
  pinMode(pushButton, INPUT);

  mySerial.begin(9600);

  // Initialising SIM808 Module
  while(!sim808.init()){
    delay(1000);
    Serial.print("Sim808 init error\r\n");
  }
  // Turning ON GPS
  if(sim808.attachGPS())
    Serial.println("Open the GPS power success");
  else 
    Serial.println("Open the GPS power failure");
}

void loop() {
  float lat, lon;
  getGPSdata(lat, lon); // Getting GPS data - latitude, longitude
  displayGPSdata( String(lat), String(lon)); // displaying the GPS data on LCD
  // waits for the push button to trigger
  char message[100];
  sprintf(message, "Help Needed!!! \nLatitude : %s\n Longitude: %s\n");
  if(digitalRead(pushButton)) {
    unsigned long trigger_time = millis();
    // code for taking run time
    Buzzer(1);
    SendSMS(phone, message);
    while(1){
      // run for 2 hrs
      if( (millis() - trigger_time)/1000 == 300) break;
      // interupt if got STOP as msg
      if(checkForSTOP())  break;

      Serial.println("struck here");
    }
    Buzzer(0);
  }

}
