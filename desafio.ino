#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN  4  
#define LED_PIN 5
#define DHTTYPE DHT11
#define TRIGGER_PIN 16
#define ECHO_PIN 17

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

WiFiClient espClient;
PubSubClient client(espClient);

char* mqttServer = "broker.hivemq.com";
int mqttPort = 1883;
char* tempTopic = "cp3/vitor/temp";
char* umiTopic = "cp3/vitor/umi";
char* respTopic = "cp3/vitor/resp";
char* distTopic = "cp3/vitor/dist";

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();
  connectToWiFi();
  connectToMQTT();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float distance = getDistance();

  Serial.println("Humidade: " + String(humidity, 2) + "%");
  Serial.println("Temperatura: " + String(temperature, 2) + "°C");
  Serial.println("Distância: " + String(distance, 2) + " cm");
  Serial.println("---");

  publishData(String(temperature, 2), tempTopic);
  publishData(String(humidity, 2), umiTopic);
  publishData(String(distance, 2), distTopic);

  client.loop();
  delay(1000);
}

void connectToWiFi() {
  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(6000);
    Serial.println("Tentando se conectar ao WiFi...");
  }
  Serial.println("WiFi conectado com sucesso!");
}

void connectToMQTT() {
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  if (client.connect("usuarioTeste")) {
    Serial.println("Conectado ao Broker MQTT");
    client.subscribe(tempTopic);
    client.subscribe(umiTopic);
    client.subscribe(respTopic);
    client.subscribe(distTopic);
  } else {
    Serial.println("Falha na conexão ao Broker MQTT");
  }
}

void publishData(String data, char* topic) {
  client.publish(topic, data.c_str());
}

float getDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  if (strcmp(topic, respTopic) == 0) {
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    Serial.println("Mensagem recebida no tópico: " + String(topic));
    Serial.println("Mensagem: " + message);
    if (message.equals("1")) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}
