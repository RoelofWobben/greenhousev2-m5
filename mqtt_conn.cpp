#include "mqtt_conn.h"

const char* MQTT_SERVER = "mosquitto.local";
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "greenhouse-m5";

WiFiClient espClientM5;
PubSubClient MqttClient(espClientM5);

bool connectMqtt() {
  MqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  MqttClient.setCallback(mqttCallback);

  Serial.println("Verbinden met MQTT .....");

  if (MqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
    Serial.println("MQTT verbonden");
    MqttClient.subscribe("greenhouse/light/status");
    return true;
  } else {
    Serial.print("MQTT verbinden mislukt, state: ");
    Serial.println(MqttClient.state());
    return false;
  }
}

void ensureMqttConnected() {
  if (!MqttClient.connected()) {
    connectMqtt();
  }
  MqttClient.loop();
}