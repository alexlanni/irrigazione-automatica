#include <Arduino.h>
#include <Ultrasonic.h>

#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>


Ultrasonic sensoreLivello = Ultrasonic(13, 15);


unsigned int minLivelloLoquido = 80;
unsigned int livelloLiquido = 0;
unsigned int sogliaMassimaRiempimento = 1000;
unsigned int livelloRiempimento = 0;


const int SensorPin = A0;
const int PompaPin = 14;
const int RiempimentoPin = 12;
bool richiediRiempimento = false;

bool AccendiPompa = false;

int soilMoistureValue = 0;

// Configurazione Accesso WIFI
const char* ssid = "AMGARDEN";
const char* password = "AleMar7981";

// Configurazione Server Mosquitto MQTT
const char* mqttServer = "192.168.1.161";

const char* clientNAme = "irrigazione_vasi";



// Configurazione WIFI Client
WiFiClient espClient;
PubSubClient client(espClient); // Necessita del Client WIFI




void callback(char* topic, byte* payload, unsigned int length) {

  Serial.println(topic);

  String p = String((char*)payload);

  Serial.println((char)payload[0]);

  if(strcmp(topic,"irr/attiva") == 0){
    Serial.println("ci sei");

    if ((char)payload[0] == '1') {
      AccendiPompa = true;   
    } 
  }

  if(strcmp(topic,"irr/riempi") == 0){
    if ((char)payload[0] == '1') {
      richiediRiempimento = true;   
    } 
  }

  Serial.println(AccendiPompa);

  

}

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
  Serial.begin(9600);

  pinMode(PompaPin, OUTPUT);
  pinMode(RiempimentoPin, OUTPUT);

  digitalWrite(PompaPin, HIGH);
  digitalWrite(RiempimentoPin, HIGH);
  

  //Setup WIFI
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
  client.setCallback(callback);
  
   // Avvio la connessione al Server MQTT
  while( !client.connected() ) {
    reconnect();
    delay(100);
  }

  client.subscribe("irr/attiva");
  client.subscribe("irr/riempi");
}

void loop() {

  client.loop();
  
  livelloLiquido = sensoreLivello.read();
  livelloRiempimento = analogRead(SensorPin);


  //Serial.println("Livello Liquido");
  Serial.println(livelloLiquido);

  //Serial.println("Livello Sensore");
  //Serial.println(livelloRiempimento);

  if(AccendiPompa){
    
    if(livelloLiquido < minLivelloLoquido){
      digitalWrite(PompaPin, LOW);
    } else {
      AccendiPompa = false; //RESET
      digitalWrite(PompaPin, HIGH);
    }
  }

  if(richiediRiempimento) {
    if(livelloRiempimento > sogliaMassimaRiempimento){
      digitalWrite(RiempimentoPin, LOW);
    } else {
      richiediRiempimento = false; // RESET
      digitalWrite(RiempimentoPin, HIGH);
    }
  }


  delay(300);
}