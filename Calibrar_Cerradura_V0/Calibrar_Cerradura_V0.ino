#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AX12A.h>

#include <ArduinoJson.h>
#include <ESP8266httpUpdate.h>

#define DirectionPin   (2u)
#define BaudRate      (1000000ul)
#define IDR        (4u)

// Datos para la conexion al WiFi

const char* ssid = "Wifisalva";                  //NOMBRE ROUTER
const char* password = "606354481";              //CONTRASEÑA
const char* mqtt_server = "192.168.1.200";

WiFiClient espClient;

PubSubClient client(espClient);

char IP_local[16];

String ID = String(ESP.getChipId());


bool Calibrar=false,Vuelta=false,Gatillo=false,zona=false,parada=false;
float reg=0,reg1=0;
float posgatillo=204;

DeserializationError error;
StaticJsonDocument<256> docCalibrar,docCon;

String jsonConS;

  void callback(char* topic, byte* payload, unsigned int length){
    char *mensaje=(char *)malloc(length+1); // reservo memoria para copia del mensaje
    strncpy(mensaje,(char*)payload,length); // copio el mensaje en cadena de caracteres
    
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
  
    for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
    }
    Serial.println();
  
    if(strcmp(topic,"TFG/Cerradura/Calibrado")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docCalibrar,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docCalibrar.containsKey("Calibrar") && docCalibrar.containsKey("Vuelta") && docCalibrar.containsKey("Gatillo") ){  // comprobar si existe el campo/clave que estamos buscando
        
        Calibrar = docCalibrar["Calibrar"];
        Vuelta = docCalibrar["Vuelta"]; 
        Gatillo = docCalibrar["Gatillo"];
          
    
        
     }else{
        Serial.println("Some keys not found");
        }
    }else{        // Si es cualquier otro topic es desconocido porque solo se suscribe este
      Serial.println("Error: Topic desconocido");
   }

  free(mensaje); // Se libera memoria

  }

  void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
      jsonConS = "";
      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(ESP.getChipId());
      docCon["CHIPID"] = ID;    
      docCon["online"] = false;
      serializeJson(docCon,jsonConS);
    // Attempt to connect
      if( client.connect(clientId.c_str(),"TFG/Cerradura/conexion", 2, true,jsonConS.c_str())){
        Serial.println("connected");
        jsonConS = "";
        docCon["CHIPID"] = ID;
        docCon["online"] = true;
        serializeJson(docCon,jsonConS);
        Serial.println(jsonConS);
        client.publish("TFG/Cerradura/conexion",jsonConS.c_str(),true);
        //...and resubscribe
        client.subscribe("TFG/Cerradura/Calibrado");
   
      }else{
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
  
  void setup_wifi(){

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

void setup(){
 
  
  
 setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  ax12a.begin(BaudRate, DirectionPin, &Serial);
  ax12a.setEndless(IDR, OFF);
  ax12a.moveSpeed(IDR,204,100);
  Serial.begin(1000000);

  delay(2000);

  }


void loop(){
  
  
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  //Prueba
 
if(Calibrar==true){
    if(Vuelta==true){
    while(parada==false){
      ax12a.setEndless(IDR, ON);
      ax12a.turn(IDR,LEFT,200);
      reg1=reg;
      reg=ax12a.readPosition(IDR);
      Serial.println(reg);
      delay(20);
    if(reg1==0 and reg==0){
    
      zona=true;
      }
  
    if(zona==true and(reg>150 and reg<204)){
      parada=true;
      ax12a.setEndless(IDR, OFF);
      ax12a.moveSpeed(IDR,204.6,150);
      reg=ax12a.readPosition(IDR);
      zona=false;}
    }
    
    Vuelta=false;
    parada=false;
    }
    
    if(Gatillo==true){
      posgatillo=posgatillo+10;
      ax12a.moveSpeed(IDR,posgatillo,200);
      Serial.println(reg);
    Gatillo=false;
      
    }
    }
  
}
