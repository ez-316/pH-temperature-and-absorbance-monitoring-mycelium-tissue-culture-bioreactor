#include <OneWire.h>
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float darksig = 0;
float lightsig = 0;
float total_signal = 0;
float total = 0;
float temp = 0;
float blank = 0;
float blanksignal = 0;

bool blanked = false;

int sensor = A0;  // output pin of OPT101 attached to Analog 0
int LEDpin = 13;    // LED + attached to D2(2)/D13(13)/D6(6)
int Buttonpin = 3; // Button attached to D3
int buttonState = 0;

int counter = 0;

// Program variables ----------------------------------------------------------
//int exampleVariable = 0;
//int sensorPin = 2;  //Data pin d2 connected to temperature sensor

// Serial data variables ------------------------------------------------------
//Incoming Serial Data Array
const byte kNumberOfChannelsFromExcel = 6; 

// Comma delimiter to separate consecutive data if using more than 1 sensor
const char kDelimiter = ',';    
// Interval between serial writes
const int kSerialInterval = 50;   
// Timestamp to track serial interval
unsigned long serialPreviousTime; 

char* arr[kNumberOfChannelsFromExcel];

void setup() {
  pinMode(Buttonpin, INPUT);
  Serial.begin(9600);
  delay(100);
  // initialize the LCD
  lcd.init();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  counter = 10;
  for (int dummy = 0; dummy < 10; dummy++)
  {

    buttonState = digitalRead(Buttonpin);
    if (buttonState == HIGH){
      counter = 20;
      for (int dummy = 0; dummy < 10; dummy++){
        lcd.setCursor(0, 0);
        lcd.print("                            ");
        lcd.setCursor(0, 1);
        lcd.print("                            ");
        lcd.setCursor(0, 0);
        lcd.print("Done in: ");
        lcd.print(counter);
        lcd.setCursor(0, 1);
        lcd.print("Blanking...");
        blanksignal = ODblank(1);
        blank = blank + blanksignal;
        
        counter = abs(counter - 1);
      }
      blanked = true;
      break;
    } else {
      blanked = false;
    }
     
    lcd.setCursor(0, 0);
    lcd.print("                            ");
    lcd.setCursor(0, 1);
    lcd.print("                            ");
    
    lcd.setCursor(0, 0);
    lcd.print("Starting in: ");
    lcd.print(counter);
    counter = abs(counter - 1);
    lcd.setCursor(0, 1);
    lcd.print("Press for blank!");
    delay(1000);
  }
  lcd.setCursor(0, 0);
  lcd.print("                                  ");
  lcd.setCursor(0, 0);
  lcd.print("Signal: ");

  lcd.setCursor(0, 1);
  lcd.print("                                  ");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  
  pinMode (LEDpin, OUTPUT); //for cell 1 LED
  delay(1000);
  
  sensors.begin();
}

void loop() {
  delay(100);
  // Gather and process sensor data
  //processSensors();
  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();
  // Process and send data to Excel via serial port (Data Streamer)
  processOutgoingSerial();
// Compares STR1 to STR2 returns 0 if true.
//   if ( strcmp ("Apple", arr[0]) == 0){ 
//       Serial.println("working");
//   }
  ODmeasurement(10);
  Serial.print(',');
  TEMPmeasurement();
  Serial.print('\n');
   
  delay(53000);  //delay off set by 7 seconds = 7000 ms already
  //delay(100);
}

float ODblank(int multiplier) {
  darksig = 0;
  digitalWrite(LEDpin, LOW); //ensures cell 1 LED is off!!!!
  for (int dummy = 0; dummy < multiplier; dummy++)
  {
    delay(100);
    darksig = abs(darksig + analogRead(sensor));
    delay(100);
  }
  delay(100);

  lightsig = 0;
  digitalWrite(LEDpin, HIGH); //turns exp LED on
  delay(1000);
  for (int dummy = 0; dummy < multiplier; dummy++)
  {
    delay(100);
    lightsig = (lightsig + analogRead(sensor));
    delay(100);
  }
  delay(100);
  digitalWrite(LEDpin, LOW); //ensures cell 1 LED is off!!!!
  delay(1000);
  
  total_signal = abs(lightsig - darksig)/multiplier;
  return total_signal;
}

void ODmeasurement(int multiplier) {
  darksig = 0;
  digitalWrite(LEDpin, LOW); //ensures cell 1 LED is off!!!!
  for (int dummy = 0; dummy < multiplier; dummy++)
  {
    delay(100);
    darksig = abs(darksig + analogRead(sensor));
    delay(100);
  }
  delay(100);

  lightsig = 0;
  digitalWrite(LEDpin, HIGH); //turns exp LED on
  delay(1000);
  for (int dummy = 0; dummy < multiplier; dummy++)
  {
    delay(100);
    lightsig = (lightsig + analogRead(sensor));
    delay(100);
  }
  delay(100);
  digitalWrite(LEDpin, LOW); //ensures cell 1 LED is off!!!!
  delay(1000);
  
  total_signal = abs(lightsig - darksig);

  

  lcd.setCursor(0, 0);
  lcd.print("                                  ");
  lcd.setCursor(0, 0);
  lcd.print("Signal: ");
  if(blanked == true){
    total = -log10(total_signal/(blank*multiplier));
    lcd.print(total);
    lcd.print(" OD");
    delay(100);
    Serial.print(total);
  }
  else {
    lcd.print(total_signal);
    delay(100);
    Serial.print(total_signal);
  }
}

void TEMPmeasurement() 
{
  delay(100) ; 
  sensors.requestTemperatures(); // Send the command to get temperature readings
  temp = sensors.getTempCByIndex(0);
  
  lcd.setCursor(0, 1);
  lcd.print("                                  ");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  Serial.print(temp);
}

// SENSOR INPUT CODE-----------------------------------------------------------
/*void processSensors() 
{
  // Read sensor pin and store to a variable
  //exampleVariable = digitalRead(sensorPin);
  
  // Add any additional raw data analysis below (e.g. unit conversions)
  
}*/

// Add any specialized methods and processing code below


// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
/*void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)
  // Repeat next 2 lines of code for each variable sent:

  //Serial.print(exampleVariable);
  //ODmeasurement(10);
  //TEMPmeasurement();
  //Serial.print(temp());
  //delay(2000);  //Time in between measurements
  //Serial.print(kDelimiter);
  //Serial.println(); // Add final line ending character only once
}*/

//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void processOutgoingSerial()
{
   // Enter into this only when serial interval has elapsed
  if((millis() - serialPreviousTime) > kSerialInterval) 
  {
    // Reset serial interval timestamp
    serialPreviousTime = millis(); 
    //sendDataToSerial(); 
  }
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void processIncomingSerial()
{
  if(Serial.available()){
    parseData(GetSerialData());
  }
}

// Gathers bytes from serial port to build inputString
char* GetSerialData()
{
  static char inputString[64]; // Create a char array to store incoming data
  memset(inputString, 0, sizeof(inputString)); // Clear the memory from a pervious reading
  while (Serial.available()){
    Serial.readBytesUntil('\n', inputString, 64); //Read every byte in Serial buffer until line end or 64 bytes
  }
  return inputString;
}

// Seperate the data at each delimeter
void parseData(char data[])
{
    char *token = strtok(data, ","); // Find the first delimeter and return the token before it
    int index = 0; // Index to track storage in the array
    while (token != NULL){ // Char* strings terminate w/ a Null character. We'll keep running the command until we hit it
      arr[index] = token; // Assign the token to an array
      token = strtok(NULL, ","); // Conintue to the next delimeter
      index++; // incremenet index to store next value
    }
}