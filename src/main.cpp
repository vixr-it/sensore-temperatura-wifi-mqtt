#include <Arduino.h>

#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>



// Configurazione Accesso WIFI
const char* ssid = "AMDomus";
const char* password = "AleMar7981";

// Configurazione Server Mosquitto MQTT
const char* mqttServer = "192.168.1.121";


// Configurazione WIFI Client
WiFiClient espClient;


// Configurazione Client MQTT
PubSubClient client(espClient); // Necessita del Client WIFI
const char* clientNAme = "vixrIT_sensor_A";

// Out Topic: Utilizzato per la pubblicazione dei messaggi
const char* outTopic1 = "vixrit/temperature/sensore/A";

// Configurazione della libreria di gestione BME280
// Costanti per il collegamento del Sensore BME280 Bosch
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

Adafruit_BME280 bme;


// Funzione di connessione/riconeessione al WIFI
void reconnect() {
  // ciclo fino a connessione effettuata
  while (!client.connected()) {
    Serial.print("Tentativo di connessione MQTT...");
    if (client.connect( clientNAme )) {  
      Serial.println("connesso");
      delay(100);
      
    } else {
      Serial.print("Fallito, rc=");
      Serial.print(client.state());
      Serial.println(" nuovo tentativo in 5 secondi");
      delay(5000);
    }
  }
}

void setup() {

  // La seriale ti serve soprattutto per il debug
  Serial.begin(115200);
  Serial.setTimeout(200000);

  // Tento la connessione al BME280
  if (! bme.begin(&Wire)) {
        Serial.println("Non riesco a trovare un sensore BME280 valido, dovresti ricontrollare i collegamenti.");
        while (1);
  }

  // Imposto la modalitá del sensore BME, seguendo la documentazione della libreria
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2, 
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_0_5 );

  // Avvio la connessione WIFI come Client
  Serial.println("Connessione alla rete WIFI" );
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Attendo affinché la connessione riesca
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println(" - ");
  Serial.print("Connesso A ");
  Serial.println(ssid);
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

  // Collegamento al server MQTT
  client.setServer(mqttServer, 1883);


  // Lettura BME
  bme.takeForcedMeasurement();

  // Avvio la connessione al Server MQTT
  while( !client.connected() ) {
    reconnect();
    delay(100);
  }

  char temp[5]; // Variabile di appoggio
  float _temp = bme.readTemperature(); // Lettura Temperatura
  dtostrf( _temp , 6,2,temp); // Riverso la temperatura letta (float) nella variabile di appoggio (char[])
  Serial.print("Rilevata temperatura, la invio a MQTT: ");
  Serial.print(temp);
  client.publish(outTopic1,  temp); // Pubblico la temperatura su MQTT

  delay(200); // Il  delay serve per far completare la transazione al client MQTT prima del DeppSleep

  Serial.println("");
  Serial.println("Vado in sleep Mode per 30 secondi");
  ESP.deepSleep(30e6); // Il valore é espresso in microsecondi


}

void loop() {
  // put your main code here, to run repeatedly:
}