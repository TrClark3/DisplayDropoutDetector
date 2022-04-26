// working version w/ wifi

// set pins
const int LDR = 33; //photoresistor // use 2 for non wifi and 33 for wifi
const int LED = 12; // led
//const int buzzer = 4; // may implement later

// sensitivity, increase/decrease to adjust photoresistor sensitivity 
int sens = 450; // started at 90 // higher = less sensitive (~500 seems okay for not flush to monitor, eventually decrease)

// initialize variables
int val = 0;
int darkness = 0;
int counter = 0;
int timeStamps[100] ={};
String incomingString;
unsigned long StartTime;

#include "DDD3_info.h" // change this to correspond to DDD you're programming
#include <WiFiClientSecure.h> // establishes secure connection to WiFi
#include <PubSubClient.h> // PubSub using MQTT Protocol 
#include <ArduinoJson.h> // For transferring json data w/ AWS IoT
#include "WiFi.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  delay(2000);
  display.clearDisplay();

  // display static text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting to Wifi..");
  display.display();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  display.clearDisplay();
  

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);
  
  // Connecting to AWS 
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting to AWS...");
  display.display();

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
  display.clearDisplay();
}

// Publishes MQTT Data to Topic "esp32/pub"
void publishMessage(int runtime, int dropouts) {
  StaticJsonDocument<200> doc;
  doc["Dropouts"] = (String) dropouts;
  doc["Runtime"] = (String) runtime + " mins";
  doc["Device"] = THINGNAME;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

// Parses incoming messages on "esp32/sub"
void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup() {
  Serial.begin(9600);
  //connectAWS(); // Comment this out for OFFLINE MODE (basic dropout count + time display)

  // initialize display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  delay(2000);
  display.clearDisplay();

  // display static text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Dropouts: ");
  display.display();
  display.setTextSize(2);
  display.setCursor(0,35);
  display.println("Minutes: ");
  display.display();

  // start time + init serial monitor timeStamp array
  StartTime = millis();
//  pinMode(buzzer, OUTPUT);
  pinMode(LED, OUTPUT);
  for (int i= 0; i<=99; i++)
        timeStamps[i]=-1;
  Serial.println("reset counter");

  // take initial light reading
  val = analogRead(LDR) - sens; 
}

void loop() {

getElapsedTime();
incomingString = "";

  //measure the resistance of the resistor repeatidly and convert it to digital value (0-4095)
  darkness = analogRead(LDR);
  delay(2);

//check for serial commands
//  if (Serial.available() > 0)
//    serialdata();

  //dropout if brightness is darker than initial reference value
  if (darkness < val) {
    counter++;
    // for serial monitor
    for (int i= 0; i<=99; i++) {
        if (timeStamps[i]!=-1);
        else {
            unsigned long CurrentTime = millis();
            int runTime = (int)(((CurrentTime - StartTime) / 1000) / 60);
            timeStamps[i]=runTime;
            Serial.print("Dropout Occured At: ");
            Serial.print(runTime);
            Serial.println("Mins");
            publishMessage(runTime, counter);
              Serial.print("reference val ");
//  Serial.print(val);
//  Serial.print("read ");
//  Serial.print(darkness);
//  Serial.print('\n'); // For debug LDR when dropout occurs
            break;
        }
    }
    
    // Display  
    String counter1=(String) counter;
    digitalWrite(LED, HIGH);
    display.setTextColor(WHITE, BLACK); // overwrites previous value
    display.setCursor(20,20);
    display.print(counter);
    display.display();
    delay(50);  

//      ledcWriteTone(buzzer, 100); // Send 100Hz sound signal (not implemented)
//      delay(400);        // for 1 sec
//      ledcWrite(buzzer,0);     // Stop sound
//      delay(10);
      
    //makes sure that the current dropout is only counted as one dropout:
    while (darkness < val ) {
      getElapsedTime(); 
      darkness=analogRead(LDR);
      digitalWrite(LED, LOW);
      delay(200); 
      digitalWrite(LED, HIGH);
      delay(200);
     
//      //check for serial commands during dropout
//      if (Serial.available() > 0)
//        serialdata();
    }
  }
  client.loop();
  delay(100);
//  Serial.print("reference val ");
//  Serial.print(val);
//  Serial.print("read ");
//  Serial.print(darkness);
//  Serial.print('\n');
//  delay(2000); //Uncomment to debug LDR issues and play with threshold
}

void getElapsedTime() {
      unsigned long CurrentTime = millis();
      int ElapsedTime = (int)(((CurrentTime - StartTime) / 1000) / 60);
      String curTime = (String)ElapsedTime;
      display.setTextColor(WHITE, BLACK); // overwrite previous value
      display.setCursor(20, 50);
      display.println(curTime);
      delay(150);
      display.display();
}

// for serial monitor control
//void serialdata() {
//  if (Serial.available() > 0) {
//    delay(50);
//    // read serial command
//    incomingString = Serial.readString();
//    incomingString.trim();
//
//    if (incomingString == "reset") {
//      counter = 0;
//      for (int i= 0; i<=99; i++)
//        timeStamps[i]=-1;
//      digitalWrite(LED, LOW);
//      display.setTextColor(WHITE, BLACK); // overwrite previous value
//      display.setCursor(20, 20);
//      display.println("0");
//      display.display();
//      Serial.println("reset successful (not timer)");
//    }
//    else if (incomingString == "d") {
//      Serial.println(counter);
//    }
//
//    else if (incomingString == "calibration") {
//      val = analogRead(LDR) - sens;
//      Serial.println("calibration finished");
//    }
//
//    else if (incomingString == "stamps") {
//      String stamps="";
//      for (int i= 0; i<=99; i++) {
//        if (timeStamps[i]!=-1)
//          stamps += String(timeStamps[i])+", ";
//        else 
//          break;
//        }    
//      Serial.println(stamps);
//    }
//
//    else if (incomingString == "t") {
//      unsigned long CurrentTime = millis();
//      double ElapsedTime = (int)(((CurrentTime - StartTime) / 1000) / 60);
//      String print1 = (String)ElapsedTime + " min";
//      Serial.println(print1);
//    }
//
//    else if (incomingString == "commands") {
//      Serial.println("reset        to reset counter (does not reset timer)");
//      Serial.println("d            get value of dropout counter");
//      Serial.println("calibration  set current brightness as default");
//      Serial.println("stamps       get time stamps of Various Dropouts (1st 100 stamps only)");
//      Serial.println("t            get time since boot up of the device");
//      Serial.println("ver          get software version");
//      Serial.println("commands     see commands");
//    }
//
//    else {
//      Serial.println("Command not found");
//    }
//  }

//}
