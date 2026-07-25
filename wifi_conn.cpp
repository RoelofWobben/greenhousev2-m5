#include "wifi_conn.h"
#include "WiFi.h"


void connectWifi() {

   Serial.print("Attempt to connect to: ");
   Serial.println(WIFI_SSID); 
   WiFi.disconnect(true, true);
   WiFi.mode(WIFI_STA);
   WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
   
    Serial.print(".");
    
    delay(1000);
  }

  Serial.println("Wifi connected");
  Serial.print("IP adres: ");
  Serial.println(WiFi.localIP());
}