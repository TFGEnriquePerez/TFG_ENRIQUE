#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Wifisalva";                  //NOMBRE ROUTER
const char* password = "606354481";              //CONTRASEÑA
const char* mqtt_server = "192.168.1.200";       //SERVIDOR
volatile int lectura = HIGH;
float inicio=0,inicio2=0,tiempo=0;
int cuenta=0;
int anterior = HIGH;
bool var1=false;
bool Emergencia=false;
StaticJsonDocument<256> docCon,docEmer,docEmer2;
DeserializationError error;
String jsonConS,docEmerS,docEmer2S;

WiFiClient espClient;

PubSubClient client(espClient);

char IP_local[16];
String ID = String(ESP.getChipId());

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
  
    if(strcmp(topic,"TFG/Boton/Emergencia2")==0){ //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
                                                   //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
    // Deserialize the JSON document
      error = deserializeJson(docEmer2,payload);
    // Compruebo si no hubo error
    if (error){ //Si ha habido error..
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else if(docEmer2.containsKey("Emergencia")){  // comprobar si existe el campo/clave que estamos buscando
        
        Emergencia = docEmer2["Emergencia"];
        if(Emergencia==true){
          cuenta=3;
          }else{cuenta=0;}
           var1=false;
           emer(Emergencia);
           
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
      if( client.connect(clientId.c_str(),"TFG/Boton/conexion", 2, true,jsonConS.c_str())){
        Serial.println("connected");
             
        jsonConS = "";
        docCon["CHIPID"] = ID;
        docCon["online"] = true;
        serializeJson(docCon,jsonConS);
        Serial.println(jsonConS);
        client.publish("TFG/Boton/conexion",jsonConS.c_str(),true);
        //...and resubscribe
           client.subscribe("TFG/Boton/Emergencia2");
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


ICACHE_RAM_ATTR void RTI(){
    lectura = digitalRead(0);
    //descomentar para eliminar rebotes
 
    if(lectura == anterior || (millis()-inicio) < 50) return;   //Filtro antirebotes para 50 ms

    Serial.println(lectura);

    if(lectura == LOW){
      anterior=LOW;
       inicio = millis();
       
    }else{
      anterior=HIGH;
      if((inicio-inicio2)<(4000)){cuenta=cuenta+1;}else{cuenta=1;}
      inicio2=millis();
      
      }
    
    
    }

void emer(bool var){
  
  
      docEmer["Emergencia"] = var;
    docEmerS = "";
    serializeJson(docEmer,docEmerS);
    
    client.publish("TFG/Boton/Emergencia",docEmerS.c_str());}

void escribir(int v1,int v2, int v3){
     digitalWrite(12,v1);
    digitalWrite(13,v2);
    digitalWrite(14,v3); 
  }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
   setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  attachInterrupt(digitalPinToInterrupt(0), RTI, CHANGE);
  Emergencia=false;
  emer(Emergencia);
  pinMode(12, OUTPUT);     
  pinMode(13, OUTPUT);  
  pinMode(14, OUTPUT); 
  
}

void loop() {
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  
  tiempo=millis();
  if(cuenta<3 && (tiempo-inicio2)>4000){cuenta=0;}

  if(cuenta>=3){
    if(var1==false){
      Emergencia=true;
    emer(Emergencia); 
    var1=true;}
    escribir(HIGH,HIGH,HIGH);
 
    
    }else if(cuenta==2 && var1==false){
      
    escribir(HIGH,HIGH,LOW);  
      }else if(cuenta==1 && var1==false){
    escribir(HIGH,LOW,LOW);
        
        
        }else if(var1==false){ 
escribir(LOW,LOW,LOW);}
 
delay(100);
}
