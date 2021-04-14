#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AX12A.h>
#include <EEPROM.h> // Manejo de la EEPROM
#include <ArduinoJson.h>
#include <ESP8266httpUpdate.h>

#define DirectionPin   (2u)
#define BaudRate      (1000000ul)
#define IDR        (4u)

// Datos para la conexion al WiFi

const char* ssid = "Wifisalva";                  //NOMBRE ROUTER
const char* password = "606354481";              //CONTRASEÑA
const char* mqtt_server = "192.168.1.200";       //SERVIDOR

WiFiClient espClient;

PubSubClient client(espClient);

char IP_local[16];
bool Calibrar=false,Vuelta=false,Gatillo=false,zona=false,parada=false,eeprom=false,program=false,fin=false,recuperar=true,Cerrar=false,Abrir=false;
float reg=-1,reg1=-1;
int posgatillo=818,resta=0,Estado=0;
int nvuelta=0;
bool SaveEeprom=false;

String ID = String(ESP.getChipId());

DeserializationError error;
StaticJsonDocument<256> docCalibrar,docVueltas,docCon,docIni,docGatillo,docPrograma,docAbrir,docCerrar,docEst;

String jsonConS,docIniS,docEstS;

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
    }else if(docCalibrar.containsKey("Calibrar")){  // comprobar si existe el campo/clave que estamos buscando
        
        Calibrar = docCalibrar["Calibrar"];
        
           
     }else{
        Serial.println("Some keys not found");
        }
    }else if(strcmp(topic,"TFG/Cerradura/Vuelta")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docVueltas,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docVueltas.containsKey("Vuelta")){  // comprobar si existe el campo/clave que estamos buscando
        
        Vuelta = docVueltas["Vuelta"];
        
           
     }else{
        Serial.println("Some keys not found");
        }
    }else if(strcmp(topic,"TFG/Cerradura/Gatillo")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docGatillo,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docGatillo.containsKey("Gatillo")){  // comprobar si existe el campo/clave que estamos buscando
        
        Gatillo= docGatillo["Gatillo"];
        
           
     }else{
        Serial.println("Some keys not found");
        }
    }else if(strcmp(topic,"TFG/Cerradura/Programa")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docPrograma,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docPrograma.containsKey("Programa")){  // comprobar si existe el campo/clave que estamos buscando
        
        program = docPrograma["Programa"];
        
           
     }else{
        Serial.println("Some keys not found");
        }
    }else if(strcmp(topic,"TFG/Cerradura/Abrir")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docAbrir,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docAbrir.containsKey("Abrir")){  // comprobar si existe el campo/clave que estamos buscando
        
        Abrir = docAbrir["Abrir"];
        
           
     }else{
        Serial.println("Some keys not found");
        }
    }else if(strcmp(topic,"TFG/Cerradura/Cerrar")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docCerrar,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docCerrar.containsKey("Cerrar")){  // comprobar si existe el campo/clave que estamos buscando
        
        Cerrar = docCerrar["Cerrar"];
        
           
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
        client.subscribe("TFG/Cerradura/Vuelta");
        client.subscribe("TFG/Cerradura/Gatillo");
        client.subscribe("TFG/Cerradura/Programa");
        client.subscribe("TFG/Cerradura/Abrir");
        client.subscribe("TFG/Cerradura/Cerrar");
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

void Inicio(){

  docIni["Inicio"] = 1;
  docIniS = "";
  serializeJson(docIni,docIniS);
  client.publish("TFG/Cerradura/Inicio",docIniS.c_str());
}

void EstadoP(){
  docEst["Estado"] = Estado;
  docEstS = "";
  serializeJson(docEst,docEstS);
  client.publish("TFG/Cerradura/Estado",docEstS.c_str());
  
}



void setup(){
   
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  ax12a.begin(BaudRate, DirectionPin, &Serial);
  ax12a.setEndless(IDR, OFF);
  ax12a.moveSpeed(IDR,818,100);
  Serial.begin(1000000);
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  Inicio();
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
          ax12a.turn(IDR,RIGHT,200);
          reg1=reg;
          reg=ax12a.readPosition(IDR);
          Serial.println(reg);
          delay(20);
    
    if(reg1==0 and reg==0){
    
      zona=true;
     }
  
    if(zona==true and(reg>818 and reg<850)){
      parada=true;
      ax12a.setEndless(IDR, OFF);
      ax12a.moveSpeed(IDR,818,150);
      reg=ax12a.readPosition(IDR);
      zona=false;}
    }
    
    Vuelta=false;
    parada=false;
    nvuelta=nvuelta+1;
    }
    
    if(Gatillo==true){
      posgatillo=posgatillo-20;
      ax12a.moveSpeed(IDR,posgatillo,200);
      reg=ax12a.readPosition(IDR);
      Serial.println(reg);
    Gatillo=false;}
 
      
     
       
      
     
   eeprom=true; 
    }else if(Calibrar==false and eeprom==true){
  
 
//EEPROM.write(0,nvuelta);
  resta=posgatillo;
 int i=1;
  while(fin==false){
    
    resta=resta-255;
    if(resta>0){
      
      //EEPROM.write(i,255);
      i=i+1;
      }else{
        resta=resta+255;
        //EEPROM.write(i,resta);
        //EEPROM.write(i+1,0);
        fin=true;
        }
    
    
    }
      ax12a.moveSpeed(IDR,818,200);
      reg=ax12a.readPosition(IDR);
      Estado=0;
   //EEPROM.write(20,Estado);
  eeprom=false;
  program=true;
  }else if(program==true){
    if(recuperar==true){
    //nvuelta=EEPROM.read(0);
int i=1;
    /*while(EEPROM.read(i)!=0){
      posgatillo=posgatillo+EEPROM.read(i);
      i++;
            
      }*/
       // Estado=EEPROM.read(20);
      Serial.println(nvuelta);
      
      recuperar=false;}

      if(Abrir==true){
        if(Estado==0){
          
          ax12a.moveSpeed(IDR,posgatillo,200);
          reg=ax12a.readPosition(IDR);
          delay(5000);
           ax12a.moveSpeed(IDR,818,200);
          reg=ax12a.readPosition(IDR); 
          EstadoP();        
          }else if(Estado==1){
            for(int i=0;i<nvuelta;i++){
               while(parada==false){
           
          ax12a.setEndless(IDR, ON);
          ax12a.turn(IDR,RIGHT,200);
          reg1=reg;
          reg=ax12a.readPosition(IDR);
          Serial.println(reg);
          delay(20);
    
    if(reg1==0 and reg==0){
    
      zona=true;
     }
  
    if(zona==true and(reg>818 and reg<850)){
      parada=true;
      ax12a.setEndless(IDR, OFF);
      ax12a.moveSpeed(IDR,818,150);
      reg=ax12a.readPosition(IDR);
      zona=false;}
    }
    
    
    parada=false;}
            
            ax12a.moveSpeed(IDR,posgatillo,200);
      reg=ax12a.readPosition(IDR);
       delay(5000);
           ax12a.moveSpeed(IDR,818,200);
          reg=ax12a.readPosition(IDR);  
           Estado=0; 
           EstadoP();}
          
        
        
        
        }
    if(Cerrar==true){
      if(Estado==0){
      for(int i=0;i<nvuelta;i++){
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
  
    if(zona==true and(reg<818 and reg>780)){
      parada=true;
      ax12a.setEndless(IDR, OFF);
      ax12a.moveSpeed(IDR,818,150);
      reg=ax12a.readPosition(IDR);
      zona=false;}
    }
    
    
    parada=false;}
    Estado=1;
    EstadoP();}
      
      
      }
    
    
    }
    

}
