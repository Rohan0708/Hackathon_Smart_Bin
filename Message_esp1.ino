/*********************************************************************************************************
S-Bin (Smart Garbage Bin)

	Statement: Monitor the filled level and the internal temperature of garbage bin
	Must have:
			1. Decide the three different filled levels (i.e., Full, Medium/half, empty) and collect the periodic data on cloud and give the three different visual 				    indications for different filled levels
			2. Sense the internal bin temperature and monitor the data on cloud.
			3. If the temperature crosses a certain threshold value, then give the warning.
	- Choose right sensors
	- Use appropriate connectivity to communicate with cloud

*********************************************************************************************************************************************/
//--------//


#include <DHT.h>
#include <DHT_U.h>
#include "ThingSpeak.h"
#include "WiFi.h"
#include <RemoteMe.h>
#include <RemoteMeSocketConnector.h>
#define DEVICE_ID 5
#define DEVICE_NAME "Message esp1"
#define TOKEN "<Token_Psd>"

RemoteMe& remoteMe = RemoteMe::getInstance(TOKEN, DEVICE_ID);

float humidity;
float temp = 0;
long T;
float distanceCM ;
#define threshold 30
#define DHTPIN 21

const int trigger = 5;
const int echo = 18;
const int LED1=22;
const int LED2=23;
const int LED3=15;

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


char ssid[] = "Wifi_Name";  //SSID
char pass[] = "Wifi_Psd";  //Password
WiFiClient client;

char* stringToSend="Some String to send";
uint16_t dataSize = 50;//size of double

uint16_t pos = 0;
uint8_t * data = (uint8_t*)malloc(dataSize );

//-------------ThingSpeak Details-----------------//
unsigned long myChannelField = 1652682; //Channel ID
const int ChannelField1 = 1; //for Temperature
const int ChannelField2 = 2; //for  Humidity
const int ChannelField3 = 3; // for Distance
const char * myWriteAPIKey = "UJ8MQ2U42VGTOS5Q"; //Your Write API Key

//-----------------WiFi Setup-------------------//
void start_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

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
//--------------------------------------------------//

void setup() {
 Serial.begin(115200);
 start_wifi();
 ThingSpeak.begin(client);
 dht.begin();
 pinMode(trigger,OUTPUT);
 pinMode(echo,INPUT);
 pinMode(LED1, OUTPUT); //led 
 pinMode(LED2, OUTPUT); //led
 pinMode(LED3, OUTPUT); //led
 remoteMe.setConnector(new RemoteMeSocketConnector()); //uses websocket connection
 //remoteMe.setDirectConnector(new RemoteMeDirectWebSocketConnector()); //uncomment if u want to enable direct connections
  remoteMe.sendRegisterDeviceMessage(DEVICE_NAME);
  
}

  void loop() { 

 remoteMe.loop();

 humidity = dht.readHumidity();  //To read Humidity
 temp = dht.readTemperature();    //To read Temperature

 if(isnan(humidity) || isnan(temp))
   {
   Serial.println("Faild to read from Sensor!\n");
   return;
   }

  //-----------------------------------------// 
  digitalWrite(trigger, LOW);
  delay(1);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  T = pulseIn(echo, HIGH);
  distanceCM = (T * 0.034)/2;
 // Serial.print("Distance in cm: ");
  //Serial.println(distanceCM);

  //------------printing Temp & Humidity Sensor values------------//
  Serial.print(" Temperature:");
  Serial.print(temp, 1);
  Serial.print(" C");
  Serial.print(" Humidity:");
  Serial.print(humidity, 1);
  Serial.print("%");
  Serial.println();
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);
  digitalWrite(LED3,LOW);

//-----------For calculation of 3 Different levels----------------//
  if (distanceCM >=2 && distanceCM <= 7) {
   Serial.print("\nfull\n");
   digitalWrite(LED1,HIGH);
     RemoteMeMessagesUtils::putString(data, pos, stringToSend);
     RemoteMeMessagesUtils::putDouble(data, pos, -1.2345);
     remoteMe.sendPushNotificationMessage(6,"S-Bin","Bin is Full!!","","","");
     remoteMe.sendUserMessage(DEVICE_ID , data, dataSize );
  }

  if (distanceCM >= 8  && distanceCM <= 12) {
    Serial.print("\nHalf\n");
    digitalWrite(LED2,HIGH);
      RemoteMeMessagesUtils::putString(data, pos, stringToSend);
      RemoteMeMessagesUtils::putDouble(data, pos, -1.2345);
      remoteMe.sendPushNotificationMessage(6,"S-Bin","Bin is nHalf!!","","","");
      remoteMe.sendUserMessage(DEVICE_ID , data, dataSize );
  }

  if (distanceCM >=13  && distanceCM <= 20) {
    Serial.print("\nEmpty\n");
    digitalWrite(LED3,HIGH);
      RemoteMeMessagesUtils::putString(data, pos, stringToSend);
      RemoteMeMessagesUtils::putDouble(data, pos, -1.2345);
      remoteMe.sendPushNotificationMessage(6,"S-Bin","Bin is empty!!","","","");
      remoteMe.sendUserMessage(DEVICE_ID , data, dataSize );
  }  

  if (distanceCM > 21) {
    Serial.print("Adujust your module\n");
      RemoteMeMessagesUtils::putString(data, pos, stringToSend);
      RemoteMeMessagesUtils::putDouble(data, pos, -1.2345);
      remoteMe.sendPushNotificationMessage(6,"S-Bin","Module might be misplaced!!","","","");
      remoteMe.sendUserMessage(DEVICE_ID , data, dataSize );
    }  

  //---------------If temperature crosses a certain threshold value, then give the warning--------------// 
  if(temp>= threshold){
    Serial.println("Temperature Threshold Reach!!");
      RemoteMeMessagesUtils::putString(data, pos, stringToSend);
      RemoteMeMessagesUtils::putDouble(data, pos, -1.2345);
      remoteMe.sendPushNotificationMessage(6,"S-Bin","Temperature Threshold Reach!! Please Take Some Action","","","");
      remoteMe.sendUserMessage(DEVICE_ID , data, dataSize );
  }

  //-------------Sending Data to ThingSpeak Cloud---------------//
  ThingSpeak.writeField(myChannelField,ChannelField2,humidity, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField,ChannelField1, temp, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField,ChannelField3,distanceCM, myWriteAPIKey); 
  delay(3000);
}
