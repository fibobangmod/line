#include<WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "--------------------------";
const char* password = "----------------------------";
const char* mqtt_server = "broker.mqttdashboard.com";
const char* mqtt_user ="-----------------------------";
const char* mqtt_password ="-----------------------------";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long now = 0;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

#define LED_BUILTIN 2

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0
#define LEDC_CHANNEL_1     1
#define LEDC_CHANNEL_2     2
#define LEDC_CHANNEL_3     3
// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13
// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     800
// drive pin
//#define LED_PIN            2
#define M1A            13
#define M1B            12
#define M2A            14
#define M2B            27
//int speed
int Outputspeed=125;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  if(value>255){value=255;}
  uint32_t duty = (8191 / valueMax) * value;
  // write duty to LEDC
  ledcWrite(channel, duty);
}

void movrun()
{
  ledcAnalogWrite(LEDC_CHANNEL_0, Outputspeed);
  ledcAnalogWrite(LEDC_CHANNEL_1, 0);
  ledcAnalogWrite(LEDC_CHANNEL_2, Outputspeed);
  ledcAnalogWrite(LEDC_CHANNEL_3, 0);
  lastMsg = now;
}
void movback()
{
  ledcAnalogWrite(LEDC_CHANNEL_0, 0);
  ledcAnalogWrite(LEDC_CHANNEL_1, Outputspeed);
  ledcAnalogWrite(LEDC_CHANNEL_2, 0);
  ledcAnalogWrite(LEDC_CHANNEL_3, Outputspeed);
  lastMsg = now;
}
void turnLeft()
{
  ledcAnalogWrite(LEDC_CHANNEL_0, Outputspeed);
  ledcAnalogWrite(LEDC_CHANNEL_1, 0);
  ledcAnalogWrite(LEDC_CHANNEL_2, 0);
  ledcAnalogWrite(LEDC_CHANNEL_3, Outputspeed);
  lastMsg = now;
}
void turnRight()
{
  ledcAnalogWrite(LEDC_CHANNEL_0, 0);
  ledcAnalogWrite(LEDC_CHANNEL_1, Outputspeed);
  ledcAnalogWrite(LEDC_CHANNEL_2, Outputspeed);
  ledcAnalogWrite(LEDC_CHANNEL_3, 0);
  lastMsg = now;
}
void StopBreak()
{
  ledcAnalogWrite(LEDC_CHANNEL_0, 255);
  ledcAnalogWrite(LEDC_CHANNEL_1, 255);
  ledcAnalogWrite(LEDC_CHANNEL_2, 255);
  ledcAnalogWrite(LEDC_CHANNEL_3, 255);
}
void Stopfree()
{
  ledcAnalogWrite(LEDC_CHANNEL_0, 0);
  ledcAnalogWrite(LEDC_CHANNEL_1, 0);
  ledcAnalogWrite(LEDC_CHANNEL_2, 0);
  ledcAnalogWrite(LEDC_CHANNEL_3, 0);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  
  if ((char)payload[0] == 'f') movrun();
  else if ((char)payload[0] == 'l') turnLeft();
  else if ((char)payload[0] == 's') StopBreak();
  else if ((char)payload[0] == 'r') turnRight();
  else if ((char)payload[0] == 'b') movback();
  else Stopfree();
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client-", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("-------------------------", "hello world");//---------------------------publish--------------
      // ... and resubscribe
      client.subscribe("--------------------------"); //------------------------subscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  // setup PINOUT mode Drive Motors---------------------------------------
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(M1A, LEDC_CHANNEL_0);
  ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(M1B, LEDC_CHANNEL_1);
  ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(M2A, LEDC_CHANNEL_2);
  ledcSetup(LEDC_CHANNEL_3, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(M2B, LEDC_CHANNEL_3);  
  Serial.begin(115200);
  delay(50);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  now = millis();
  if (now - lastMsg > 1000) Stopfree();
}
