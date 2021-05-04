#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include <AX12A.h>

#include <EEPROM.h> // Manejo de la EEPROM

#include <ArduinoJson.h>

#include <ESP8266httpUpdate.h>

//Parametros Servomotor

#define DirectionPin(2 u)
#define BaudRate(1000000 ul)
#define IDR(4 u)

// Datos para la conexion al WiFi

const char * ssid = "Wifisalva"; //NOMBRE ROUTER
const char * password = "606354481"; //CONTRASEÑA
const char * mqtt_server = "192.168.1.200"; //SERVIDOR

WiFiClient espClient;

PubSubClient client(espClient);

char IP_local[16];
bool Calibrar = false, Vuelta = false, Gatillo = false, zona = false, parada = false, eeprom = false, program = false, fin = false, recuperar = true, Cerrar = false, Abrir = false;
bool Emergencia = false, EmerVar = true;
float reg = -1, reg1 = -1;
int posgatillo = 0, resta = 0, Estado = 0;
int nvuelta = 0;
bool SaveEeprom = false;
float Bateria;
int EstadoV;
bool conf = true;
String ID = String(ESP.getChipId());
int control = 0;

float sinVal;
int toneVal;

DeserializationError error;
StaticJsonDocument < 256 > docCalibrar, docVueltas, docCon, docIni, docGatillo, docPrograma, docAbrir, docCerrar, docEst, docEmergencia;

String jsonConS, docIniS, docEstS;

void callback(char * topic, byte * payload, unsigned int length) {
   char * mensaje = (char * ) malloc(length + 1); // reservo memoria para copia del mensaje
   strncpy(mensaje, (char * ) payload, length); // copio el mensaje en cadena de caracteres

   Serial.print("Message arrived [");
   Serial.print(topic);
   Serial.print("]: ");

   for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
   }
   Serial.println();

   if (strcmp(topic, "TFG/Cerradura/Calibrado") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docCalibrar, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docCalibrar.containsKey("Calibrar")) { // comprobar si existe el campo/clave que estamos buscando

         Calibrar = docCalibrar["Calibrar"];

      } else {
         Serial.println("Some keys not found");
      }
   } else if (strcmp(topic, "TFG/Cerradura/Vuelta") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docVueltas, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docVueltas.containsKey("Vuelta")) { // comprobar si existe el campo/clave que estamos buscando

         Vuelta = docVueltas["Vuelta"];

      } else {
         Serial.println("Some keys not found");
      }
   } else if (strcmp(topic, "TFG/Cerradura/Gatillo") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docGatillo, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docGatillo.containsKey("Gatillo")) { // comprobar si existe el campo/clave que estamos buscando

         Gatillo = docGatillo["Gatillo"];

      } else {
         Serial.println("Some keys not found");
      }
   } else if (strcmp(topic, "TFG/Cerradura/Programa") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docPrograma, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docPrograma.containsKey("Programa")) { // comprobar si existe el campo/clave que estamos buscando

         program = docPrograma["Programa"];

      } else {
         Serial.println("Some keys not found");
      }
   } else if (strcmp(topic, "TFG/Cerradura/Abrir") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docAbrir, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docAbrir.containsKey("Abrir")) { // comprobar si existe el campo/clave que estamos buscando

         Abrir = docAbrir["Abrir"];

      } else {
         Serial.println("Some keys not found");
      }
   } else if (strcmp(topic, "TFG/Cerradura/Cerrar") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docCerrar, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docCerrar.containsKey("Cerrar")) { // comprobar si existe el campo/clave que estamos buscando

         Cerrar = docCerrar["Cerrar"];

      } else {
         Serial.println("Some keys not found");
      }
   } else if (strcmp(topic, "TFG/Cerradura/Emergencia") == 0) { //Topic de configuracion que informa sobre el el periodo de envío de mensajes (en segundos), 
      //el periodo de comprobación de actualizaciones en minutos y la velocidad de cambio de la salida PWM 
      // Deserialize the JSON document
      error = deserializeJson(docEmergencia, payload);
      // Compruebo si no hubo error
      if (error) { //Si ha habido error..
         Serial.print("Error deserializeJson() failed: ");
         Serial.println(error.c_str());
      } else if (docEmergencia.containsKey("Emergencia")) { // comprobar si existe el campo/clave que estamos buscando

         Emergencia = docEmergencia["Emergencia"];

      } else {
         Serial.println("Some keys not found");
      }
   } else { // Si es cualquier otro topic es desconocido porque solo se suscribe este
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
      serializeJson(docCon, jsonConS);
      // Attempt to connect
      if (client.connect(clientId.c_str(), "TFG/Cerradura/conexion", 2, true, jsonConS.c_str())) {
         Serial.println("connected");
         Abrir = false;
         Cerrar = false;

         jsonConS = "";
         docCon["CHIPID"] = ID;
         docCon["online"] = true;
         serializeJson(docCon, jsonConS);
         Serial.println(jsonConS);
         client.publish("TFG/Cerradura/conexion", jsonConS.c_str(), true);
         //...and resubscribe
         client.subscribe("TFG/Cerradura/Calibrado");
         client.subscribe("TFG/Cerradura/Vuelta");
         client.subscribe("TFG/Cerradura/Gatillo");
         client.subscribe("TFG/Cerradura/Programa");
         client.subscribe("TFG/Cerradura/Abrir");
         client.subscribe("TFG/Cerradura/Cerrar");
         client.subscribe("TFG/Cerradura/Emergencia");
      } else {
         Serial.print("failed, rc=");
         Serial.print(client.state());
         Serial.println(" try again in 5 seconds");
         // Wait 5 seconds before retrying
         delay(5000);
      }
   }
}

void setup_wifi() {

   delay(10);
   // We start by connecting to a WiFi network
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }

   randomSeed(micros());

   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
}

void Inicio() {

   docIni["Inicio"] = 1;
   docIniS = "";
   serializeJson(docIni, docIniS);
   client.publish("TFG/Cerradura/Inicio", docIniS.c_str());
}

void EstadoP() {
   EEPROM.write(20, Estado);
   EEPROM.write(21, 0);
   EEPROM.commit();
   docEst["Estado"] = Estado;
   docEstS = "";
   serializeJson(docEst, docEstS);
   client.publish("TFG/Cerradura/Estado", docEstS.c_str());

}

void VueltaC(bool Sentido, int i) {
   while (parada == false) {

      reg1 = reg;
      ax12a.setEndless(IDR, ON);
      ax12a.turn(IDR, Sentido, 200);
      reg = ax12a.readPosition(IDR);

      delay(20);

      if (reg1 == 0 and reg == 0) {

         zona = true;
         if (control == 0) {
            EEPROM.write(21, i);
            EEPROM.commit();
            
            control++;
         }
      }

      if (zona == true and((reg > 818 and reg < 850) || (reg > 790 and reg < 818))) {
         parada = true;
         MovPos(818);
         zona = false;
      }
   }
   control = 0;

}
void MovPos(int Pos) {

   ax12a.setEndless(IDR, OFF);
   ax12a.moveSpeed(IDR, Pos, 150);
   reg = ax12a.readPosition(IDR);

}

void configuracion() {

   nvuelta = EEPROM.read(0);

   int i = 1;

   while (EEPROM.read(i) != 0) {

      posgatillo = posgatillo + EEPROM.read(i);

      i++;

   }
   Estado = EEPROM.read(20);
   EstadoV = EEPROM.read(21);
   Serial.println(EEPROM.read(20));
   Serial.println(EEPROM.read(21));
   delay(1000);

   if (Estado == 0) {

      for (int j = EstadoV; j < nvuelta; j++) {

         VueltaC(LEFT, j);
         Serial.println("Heeeeey estoy aki");
         parada = false;
      }
      Estado = 1;
   } else if (Estado == 1) {
      for (int j = 1; j <= EstadoV; j++) {
         Serial.println("Heeeeey no estoy aki");
         VueltaC(LEFT, j);

         parada = false;
      }
      Estado = 1;
   }
   EstadoP();

}

void setup() {

   setup_wifi();
   client.setServer(mqtt_server, 1883);
   client.setCallback(callback);
   ax12a.begin(BaudRate, DirectionPin, & Serial);
   EEPROM.begin(30);
   Serial.begin(1000000);
   MovPos(818);
   pinMode(14, OUTPUT);
   if (!client.connected()) {
      reconnect();
   }
   client.loop();
   Inicio();
   delay(2000);

}

void loop() {
   
   Bateria = (analogRead(A0) / 67.6);
   
   if (!client.connected()) {
      reconnect();
   }
   client.loop();

   if (Bateria > 11.4) {

      if (Calibrar == true) {

         if (Vuelta == true) {

            VueltaC(RIGHT, 0);
            Vuelta = false;
            parada = false;
            nvuelta = nvuelta + 1;
         }

         if (Gatillo == true) {
            posgatillo = ax12a.readPosition(IDR);
            posgatillo = posgatillo - 20;
            MovPos(posgatillo);
            Gatillo = false;
         }
         conf = false;
         eeprom = true;

      } else if (Calibrar == false and eeprom == true) {

         EEPROM.write(0, nvuelta);

         resta = posgatillo;
         int i = 1;

         while (fin == false) {

            resta = resta - 255;

            if (resta > 0) {

               EEPROM.write(i, 255);

               i = i + 1;

            } else {

               resta = resta + 255;
               EEPROM.write(i, resta);
               EEPROM.write(i + 1, 0);

               fin = true;

            }
         }
         MovPos(818);
         Estado = 0;
         EstadoP();
         eeprom = false;
         program = true;
      } else if (Emergencia == true) {
         
         tone(14, 2000);
      
         if (EmerVar == true) {
            if (Estado == 1) {

               MovPos(posgatillo);
               for (int i = 1; i <= nvuelta; i++) {

                  VueltaC(RIGHT, i);
                  parada = false;
               }
               MovPos(posgatillo);
               Estado = 0;
               EstadoP();
            }
            if (Estado == 0) {

               MovPos(posgatillo);
            }

            EmerVar = false;
         }
            
         for (int x = 0; x < 180; x++) {
            // convertimos grados en radianes para luego obtener el valor.
            sinVal = (sin(x * (3.1412 / 180)));
            // generar una frecuencia a partir del valor sin
            toneVal = 1900 + (int(sinVal * 1000));
            tone(14, toneVal);
            delay(2);
         }

      } else if (program == true && Emergencia == false) {
         if (EmerVar == false) {
            tone(14, 0);
            MovPos(818);
            EmerVar = true;
         }
         if (conf == true) {
            configuracion();

            conf = false;
         }
         if (Abrir == true) {

            if (Estado == 0) {

               MovPos(posgatillo);
               delay(5000);
               MovPos(818);
               EstadoP();

            } else if (Estado == 1) {

               for (int i = 1; i <= nvuelta; i++) {

                  VueltaC(RIGHT, i);
                  parada = false;
               }

               MovPos(posgatillo);
               delay(5000);
               MovPos(818);
               Estado = 0;
               EstadoP();
            }

         }
         if (Cerrar == true) {

            if (Estado == 0) {

               for (int i = 1; i <= nvuelta; i++) {

                  VueltaC(LEFT, i);
                  parada = false;
               }

               Estado = 1;
               EstadoP();
            }
         }
      }
   } else {

      Serial.println("Cargue bateria");
      delay(2000);

   }

   delay(200);

}
