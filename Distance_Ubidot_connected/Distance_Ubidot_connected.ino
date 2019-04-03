/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>

/****************************************
 * Define Constants
 ****************************************/
namespace {
 #define WIFISSID "Ariel_University" // Put your WifiSSID here
#define PASSWORD "" // Put your wifi password here
#define TOKEN "BBFF-75uFXzPDgmgSp1Is3IeNeRWYfzxiH8" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "ZDRFEDPAVO" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices
 
  const char * VARIABLE_LABEL = "distance"; // Assing the variable label
  const char * DEVICE_LABEL = "esp32"; // Assig the device label
  const char * MQTT_BROKER = "industrial.api.ubidots.com";  
  const int trigPin = 12; // Triger pin of the HC-SR04
  const int echoPin = 13; // Echo pin of the HC-SR04  
}

/* Sensor's declarations */
long duration;
float distance;
/* Space to store the request */
char payload[300];
char topic[150];
/* Space to store values to send */
char str_sensor[10];

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

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
  WiFi.begin(WIFISSID, PASSWORD);

  /* Assign the PINS as INPUT/OUTPUT */
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
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
