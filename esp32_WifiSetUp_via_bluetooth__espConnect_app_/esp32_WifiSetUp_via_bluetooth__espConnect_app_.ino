

/****************************************
 * Include Libraries
 ****************************************/
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
/****************************************
 * Define Constants
 ****************************************/
namespace {
// #define WIFISSID "LYA" // Put your WifiSSID here
//#define PASSWORD "65643083" // Put your wifi password here
#define TOKEN "BBFF-zIOTwedsfKj84fXarZXHmdgJaAcSBT" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "ZDRFEDPAVO" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices
 
  const char * VARIABLE_LABEL = "volume"; // Assing the variable label
  const char * DEVICE_LABEL = "sausmart"; // Assig the device label
  const char * MQTT_BROKER = "industrial.api.ubidots.com";  
  const int trigPin = 12; // Triger pin of the HC-SR04
  const int echoPin = 13; // Echo pin of the HC-SR04  
}
/* wifi ssid and password variables */
BluetoothSerial SerialBT;
//char WIFISSID [10], PASSWORD [10];
//bool wifiSetted = false ;
/* Sensor's declarations */
long duration;
float distance;
/* Space to store the request */
char payload[300];
char topic[150];
/* Space to store values to send */
char str_sensor[10];

bool ssidAndpwSetted;
bool isPaired;


/* space to store ssid ans pw*/
String ssid = "null";
String pw = "null";

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

/**
 * initBTSerial
 * Initialize Bluetooth Serial
 * Start BLE server and service advertising
 * @return <code>bool</code>
 *      true if success
 *      false if error occured
 */
bool initBTSerial() {
    if (!SerialBT.begin("ESP32")) {
      Serial.println("Failed to start BTSerial");
      return false;
    }
  Serial.println("The device started.\nPlease use EspConnect app to pair it with bluetooth and set SSID and password of wifi!");  
  return true;
}

// detect if the two devices are paired.
void pairedBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
        isPaired = true ;
    Serial.println("\nPaired !");
  }
}

/**
 * wifi_setup
 * wait for wifi set up from the espConnect app
 */
void wifi_setup() {

  while(WiFi.status() != WL_CONNECTED) 
   {
    ssidAndpwSetted = false ;
    Serial.println("Please set Wifi");
    while(!ssidAndpwSetted)
      {
       if(setSsid_Pw()) 
       {
        ssidAndpwSetted = true ; 
        Serial.println("Wifi Setted !");
        Serial.println("SSID: "+ssid+"\npassword: "+pw);
       }
     }
     WiFi.begin(ssid.c_str(),pw.c_str());
      
    Serial.println();
    Serial.print("Wait for WiFi...");
    uint64_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);

      switch (WiFi.status()){
        case WL_CONNECT_FAILED:
          Serial.println("Connection failed.");
          WiFi.disconnect();
          pw = "null"; 
          break;      
        break;
        case WL_NO_SSID_AVAIL:
        Serial.println("No ssid available.");
          WiFi.disconnect();
          pw = "null";
          break;
        break;
      }
     // if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_NO_SSID_AVAIL || 
     if(millis() - start > 15000)
        {   
          Serial.println("time out.");
          WiFi.disconnect();
          pw = "null";
          break;         
        }
    }
  }
}
/**
 * setSsid_Pw 
 * read all data from BTSerial receive buffer
 * parse data for valid WiFi credentials
 */
bool setSsid_Pw() {
  if (SerialBT.available()) {
    uint64_t startTimeOut = millis();
    String receivedData;
    int msgSize = 0;
    // Read RX buffer into String
    while (SerialBT.available() != 0) {
      receivedData += (char)SerialBT.read();
      msgSize++;
      // Check for timeout condition
      if ((millis()-startTimeOut) >= 5000) break;
    }
    SerialBT.flush();
    
    Serial.println("Received message " + receivedData + " over Bluetooth");
  
  // decode the message
    if(receivedData.substring(0,1) == "S"){
          ssid = receivedData.substring(5);
      }
      
      if(receivedData.substring(0,1) == "P"){
          pw = receivedData.substring(3);
      }
  }    
if(ssid.equals("null") || pw.equals("null"))
  return false;
else 
  return true ;
}

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

/****************************************
 * Sensor Functions
 ****************************************/
float readDistance() {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = (pulseIn(echoPin, HIGH)); 
  distance = float(duration/29/2);
  return distance; 
}


/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);

  /* Assign the PINS as INPUT/OUTPUT */
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

 // to detect if the devices are paired.
  SerialBT.register_callback(pairedBT);
  isPaired = false ;
  initBTSerial();

//  ssidAndpwSetted = false ;
//  while(!ssidAndpwSetted)
//    {
//     if(setSsid_Pw()) 
//     {
//      ssidAndpwSetted = true ; 
//      Serial.println("Wifi Setted !");
//      Serial.println("SSID: "+ssid+" password: "+pw);
//     }
//    }

Serial.print("Wait for bluetooth pairing..");
while (!isPaired) {
    Serial.print(".");
    delay(500);
  }
  
  wifi_setup();
  
//  WiFi.begin(ssid.c_str(),pw.c_str());
//    
//  Serial.println();
//  Serial.print("Wait for WiFi...");
//  
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print(".");
//    delay(500);
//  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  /* call the funtion readDistance() */
  distance = readDistance();
  
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(distance, 4, 2, str_sensor);
  
  /* Building the Ubidots request */
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value

 // sprintf(payload, "%s: %s", VARIABLE_LABEL, str_sensor); // Adds the variable label
  //sprintf(payload, "%s {"value": %s}}", payload, str_sensor); 
  /* Print the sensor reading to the Serial Monitor */
  Serial.println("Publishing values to Ubidots Cloud");
  Serial.print("Distance = ");
  Serial.println(distance);
  
  /* Publish the request to Ubidots */
   client.publish(topic, payload);
  client.loop();
  delay(1000);
}
