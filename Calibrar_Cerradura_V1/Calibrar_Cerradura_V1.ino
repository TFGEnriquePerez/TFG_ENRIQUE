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
float posgatillo=204.6;

DeserializationError error;
StaticJsonDocument<256> docCalibrar,docCon;

String jsonConS;

  void callback(char* topic, byte* payload, unsigned int length){
    char *mensaje=(char *)malloc(length+1); // reservo memoria para copia del mensaje
    strncpy(mensaje,(char*)payload,length); // copio el mensaje en cadena de caracteres
   
    for (int i = 0; i < length; i++){
 
    }

    if(strcmp(topic,"TFG/Cerradura/Calibrado")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docCalibrar,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
     
    }else if(docCalibrar.containsKey("Calibrar") && docCalibrar.containsKey("Vuelta") && docCalibrar.containsKey("Gatillo") ){  // comprobar si existe el campo/clave que estamos buscando
        
        Calibrar = docCalibrar["Calibrar"];
        Vuelta = docCalibrar["Vuelta"]; 
        Gatillo = docCalibrar["Gatillo"];
          
    
        
     }else{
    
        }
    }else{        // Si es cualquier otro topic es desconocido porque solo se suscribe este
     
   }

  free(mensaje); // Se libera memoria

  }

  void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
      jsonConS = "";
   
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(ESP.getChipId());
      docCon["CHIPID"] = ID;    
      docCon["online"] = false;
      serializeJson(docCon,jsonConS);
    // Attempt to connect
      if( client.connect(clientId.c_str(),"TFG/Cerradura/conexion", 2, true,jsonConS.c_str())){
       
        jsonConS = "";
        docCon["CHIPID"] = ID;
        docCon["online"] = true;
        serializeJson(docCon,jsonConS);
       
        client.publish("TFG/Cerradura/conexion",jsonConS.c_str(),true);
        //...and resubscribe
        client.subscribe("TFG/Cerradura/Calibrado");
   
      }else{
           // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
  
  void setup_wifi(){

    delay(10);
    // We start by connecting to a WiFi network
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      
    }

    randomSeed(micros());

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

  
 while(Calibrar==true){
    if(Vuelta==true){
    while(parada==false){
      ax12a.setEndless(IDR, ON);
      ax12a.turn(IDR,LEFT,150);
      reg1=reg;
      reg=ax12a.readPosition(IDR);
      
      delay(20);
    if(reg1>reg){
    
      zona=true;
      }
  
    if(zona==true and(reg==202 or reg==203 or reg==204 or reg==205 or reg==206)){
      parada=true;
      ax12a.setEndless(IDR, OFF);
      ax12a.moveSpeed(IDR,204.6,200);
      zona=false;}
    }
    
    Vuelta=false;
    
    }
    if(Gatillo==true){
      posgatillo=posgatillo+1;
      ax12a.moveSpeed(IDR,posgatillo,200);
    Gatillo=false;
      
    }
    }
  
}
