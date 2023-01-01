#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int button [4] = {5, 4, 0, 2}; //D1, D2, D3, D4  //5, 4, 0, 2
int button_status [4] = {LOW, LOW, LOW, LOW};

//int led [4] = {16, 3, 1, 10}; //D0, RX, TX, SD3

int port [4] = {15, 13, 12, 14}; //D5, D6, D7, D8 //14, 12, 13, 15
int port_status [4] = {LOW, LOW, LOW, LOW};

const char* ssid = "WiFi - SSID";
const char* password = "WiFi - Password";

const char* mqtt_server = "MQTT server";
const String mqtt_user = "MQTT username";
const String mqtt_password = "MQTT password";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (256)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.
  println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    yield();
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void publishStatus() {
  snprintf (msg, MSG_BUFFER_SIZE, "{\n  1: %s,\n  2: %s,\n  3: %s,\n  4: %s\n}", port_status[0] == LOW ? "OFF" : "ON", port_status[1] == LOW ? "OFF" : "ON", port_status[2] == LOW ? "OFF" : "ON", port_status[3] == LOW ? "OFF" : "ON");
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("/cord/status", msg);
}

void setPort(int _port, int _status) {
  Serial.print("Setting port "); Serial.print(_port); Serial.print(" to "); Serial.println(_status);
  if ((_status == LOW || _status == HIGH) && _port >= 0 && _port < 4) {
    port_status[_port] = _status; 
    digitalWrite(port[_port], port_status[_port]);
    //digitalWrite(led[_port], port_status[_port]);
  }
  publishStatus();
}

int determineOnOrOff(byte* payload, unsigned int length) {
  if (length == 4) {
    String tmp = "";
    tmp += (char)payload[2];
    tmp += (char)payload[3];
    if (tmp == "ON") {
      return 1;
    }
  }
  if (length == 5) {
    String tmp = "";
    tmp += (char)payload[2];
    tmp += (char)payload[3];
    tmp += (char)payload[4];
    if (tmp == "OFF") {
      return 0;
    }
  }
  if (length == 6) {
    String tmp = "";
    tmp += (char)payload[0];
    tmp += (char)payload[1];
    tmp += (char)payload[2];
    tmp += (char)payload[3];
    tmp += (char)payload[4];
    tmp += (char)payload[5];
    if (tmp == "STATUS") {
      return 2;
    }
  }
  return -1;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  int onOffStatus = determineOnOrOff(payload, length);
  if (onOffStatus == 0 || onOffStatus == 1) {
    setPort(payload[0]-'0', (onOffStatus == 1) ? HIGH : LOW);
  } else
  if (onOffStatus == 2) {
    publishStatus();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("/cord/status", "hello world");
      // ... and resubscribe
      client.subscribe("/cord/update");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void updateButtons() {
  for (int i = 0; i < 4; i++) {
    button_status[i] = digitalRead(button[i]);
    yield();
  }
}

boolean isButtonPressed() {
  for (int i = 0; i < 4; i++) {
    if (button_status[i] == LOW) {
      return true;
    }
  }
  return false;
}

void handleButtonPressed() {
  for (int i = 0; i < 4; i++) {
    if (button_status[i] == LOW) {
      Serial.print("Button pressed: ");
      Serial.println(i);
      setPort(i, port_status[i] == LOW ? HIGH : LOW);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  yield();
  delay(100);
  yield();

  for (int i = 0; i < 4; i++) {
    pinMode(button[i], INPUT);
    pinMode(port[i], OUTPUT);
    //pinMode(led[i], OUTPUT);
    yield();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  updateButtons();

  yield();

  if (isButtonPressed()) {
    handleButtonPressed();
    do {
      updateButtons();
      yield();
    } while (isButtonPressed());
  }
}
