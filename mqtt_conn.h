#pragma once

#include "WiFi.h"
#include <PubSubClient.h>
#include "secrets.h"
#include "ca_cert.h"
#include "WiFiClientSecure.h"

extern WiFiClientSecure espClientM5;
extern PubSubClient MqttClient;

bool connectMqtt();
void ensureMqttConnected();

// Gedefinieerd in greenhousev2-m5.ino, want die past lightPanel.state aan
void mqttCallback(char* topic, byte* payload, unsigned int length);