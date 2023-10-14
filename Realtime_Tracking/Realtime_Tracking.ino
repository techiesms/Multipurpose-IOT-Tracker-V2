#define DEBUG true


//******************* Pin Configurations *******************//

#define A9G_PON     14
#define A9G_POFF    13
#define A9G_LOWP    27
#define SOS_Button 22


//******************* MQTT Parameters *******************//

String MQTT_BROKER = "";
String MQTT_PORT = "1883";
String MQTT_USERNAME = "";
String MQTT_PASSWORD = "";


//******************* SIM Paramaters *******************//

String SOS_NUM = "";
String APN_NAME = "";


//******************* Data Sending Interval *******************//
uint16_t Send_Data_After = 60; // 60 sec waiting
uint8_t SOS_Time = 5; // Press the button 5 sec

//******************* Necessary Variables *******************//

String fromGSM = "";
String res = "";
char* response = " ";
String location_data;
String lats;
String longi;
String link1;
String link2;
String msg;

// define two tasks for MQTT_Task & SOSButton_Task
void MQTT_Task( void *pvParameters );
void SOSButton_Task( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup()
{


  Serial.begin(115200); // For ESP32
  Serial1.begin(115200, SERIAL_8N1, 33, 25); //For A9G

  pinMode(A9G_PON, OUTPUT);//LOW LEVEL ACTIVE
  pinMode(A9G_POFF, OUTPUT);//HIGH LEVEL ACTIVE
  pinMode(A9G_LOWP, OUTPUT);//LOW LEVEL ACTIVE
  pinMode(SOS_Button, INPUT);

  digitalWrite(A9G_POFF, LOW);
  digitalWrite(A9G_LOWP, HIGH);
  digitalWrite(A9G_PON, HIGH);
  delay(1000);
  digitalWrite(A9G_PON, LOW);
  

  msg = "";
  msg = sendData("AT+RST=1", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+RST = 1", 2000, DEBUG);
    Serial.println("Trying");
  }

  Serial.println("Before Delay");
  delay(6000);// Waiting For 15 Sec for Initialisation

  Serial.println("After Delay");

  msg = "";
  msg = sendData("AT", 1000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT", 1000, DEBUG);
    Serial.println("Trying");
  }

  msg = "";
  msg = sendData("AT+MQTTDISCONN", 1000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+MQTTDISCONN", 1000, DEBUG);
    Serial.println("Trying");
  }

  msg = "";
  msg = sendData("AT+GPS=1", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+GPS=1", 1000, DEBUG);
    Serial.println("Trying");
  }

  // GPS low power
  msg = "";
  msg = sendData("AT+GPSLP = 2", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+GPSLP = 2", 1000, DEBUG);
    Serial.println("Trying");
  }

  msg = "";
  msg = sendData("AT+CGATT=1", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+CGATT=1", 1000, DEBUG);
    Serial.println("Trying");
  }

  msg = "";
  msg = sendData("AT+CGDCONT=1,\"IP\",\"" + APN_NAME + "\"", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+CGDCONT=1,\"IP\",\"" + APN_NAME + "\"", 1000, DEBUG);
    Serial.println("Trying");
  }


  msg = "";
  msg = sendData("AT+CGACT=1", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+CGACT=1", 1000, DEBUG);
    Serial.println("Trying");
  }

  // For Speaker
  msg = "";
  msg = sendData("AT+SNFS=2", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+SNFS=2", 1000, DEBUG);
    Serial.println("Trying");
  }

  msg = "";
  msg = sendData("AT+CLVL=8", 2000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+CLVL=8", 1000, DEBUG);
    Serial.println("Trying");
  }

  msg = "";
  msg = sendData("AT+MQTTCONN=\"" + MQTT_BROKER + "\"," + MQTT_PORT + ",\"ABCD\",120,0,\"" + MQTT_USERNAME + "\",\"" + MQTT_PASSWORD + "\"", 3000, DEBUG);
  while ( msg.indexOf("OK") == -1 ) {
    msg = sendData("AT+MQTTCONN=\"" + MQTT_BROKER + "\"," + MQTT_PORT + ",\"ABCD\",120,0,\"" + MQTT_USERNAME + "\",\"" + MQTT_PASSWORD + "\"", 1000, DEBUG);
    Serial.println("Trying");
  }

  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    MQTT_Task
    ,  "MQTT_Task"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  0);

  xTaskCreatePinnedToCore(
    SOSButton_Task
    ,  "SOSButton_Task"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL
    ,  0);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void MQTT_Task(void *pvParameters)  // This is a task.
{
  (void) pvParameters;


  for (;;) // A Task shall never return or exit.
  {
    Serial1.println("AT+LOCATION=2\r\n");
    delay(2000);
    while (!Serial1.available())
    {
      Serial.println("Waiting");
      delay(10);
    }
    while (Serial1.available())
    {
      char add = Serial1.read();
      res = res + add;
      delay(1);
    }
    response = &res[0];

    if (strstr(response, "GPS NOT"))
    {
      Serial.println("No Location data");
    }
    else
    {

      int j = 0;
      while (res[j] != '2')
        j++;

      res = res.substring(j + 5);
      int k = 0;
      while (res[k] != '\n')
        k++;
      res = res.substring(0, k);
      response = &res[0];

      Serial.print("Current String -"); Serial.print(response); Serial.println("-");
      int i = 0;
      while (response[i] != ',')
        i++;

      location_data = (String)response;
      lats = location_data.substring(0, i);
      longi = location_data.substring(i + 1, i + 10);
      Serial.print("Lat - "); Serial.print(lats); Serial.println("-");
      Serial.print("Longi - "); Serial.print(longi); Serial.println("-");
      int longi_length = longi.length();
      Serial.print("Longi Length - "); Serial.print(longi_length); Serial.println("-");
      //lats.trim();

      String coordinates = "0," + lats + "," + longi.c_str() + ",0";
      Serial.print("coordinates - "); Serial.print(coordinates); Serial.println("-");

      link1 = ("AT+MQTTPUB=\"Sachin_SMS/feeds/GPSLOC/csv\",\"" + coordinates + "\",0,0,0") ;
      //Serial1.println("AT+MQTTPUB=\"A9G\",\"12.34567890123456789.0123456\",0,0,0");
      Serial.print("link lat -"); Serial.println(link1);

      //Serial.print("For Serial Monitor-"); Serial.println(link1);

      // Serial1.println(link1);
      sendData(link1, 1000, DEBUG);

      delay(2000);

      Serial.println("Location DataSend");

    }

    msg = "";
    msg = sendData("AT+CBC?", 2000, DEBUG);
    while ( msg.indexOf("OK") == -1 ) {
      msg = sendData("AT+CBC?", 1000, DEBUG);
      Serial.println("Trying");
    }
    Serial.print("Recevied Data Before - "); Serial.println(msg); // printin the String in lower character form
    int count = 0;
    while (msg[count] != ',')
    {
      count++;
      Serial.print(msg[count]);

    }

    msg = msg.substring(count + 2);

    count = 0;
    while (msg[count] != '\n')
    {
      count++;
      Serial.print(msg[count]);

    }

    msg = msg.substring(0, count - 1);

    Serial.print("Recevied Data - "); Serial.println(msg); // printin the String in lower character form
    Serial.println("\n");

    link2 = ("AT+MQTTPUB=\"Sachin_SMS/feeds/battery\",\"" + msg + "\",0,0,0") ;
    Serial.print("battery link -"); Serial.println(link2);
    Serial.print("For Serial Monitor-"); Serial.println(link2);

    // Serial1.println(link1);
    msg = "";
    msg =  (sendData(link2, 1000, DEBUG));
    char* msg_char = &msg[0];
    Serial.print("LAT MSG - "); Serial.println(msg_char);
    if ( !(strstr(msg_char, "OK" ))) {
     // MQTT_ReConnect();
    }
    delay(2000);

    Serial.println("Battery DataSend");

    response = "";
    res = "";


    Serial.println("Delay");

    vTaskDelay((Send_Data_After * 1000));
  }
}

void SOSButton_Task(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  /*
    AnalogReadSerial
    Reads an analog input on pin A3, prints the result to the serial monitor.
    Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
    Attach the center pin of a potentiometer to pin A3, and the outside pins to +5V and ground.

    This example code is in the public domain.
  */

  for (;;)
  {

    if (digitalRead(SOS_Button) == LOW)
    {
      Serial.println("Button Pressed");
      for (int l = 0; l <= SOS_Time; l++)
      {
        Serial.println(l);
        if (l == 5 && digitalRead(SOS_Button) == LOW)
        {
          Serial.println("SOS Triggered");
          Get_gmap_link(1);
          l = 0;
        }
        if (digitalRead(SOS_Button) == HIGH)
          break;
        vTaskDelay(1000);
      }
    }
    vTaskDelay(10);  // one tick delay (15ms) in between reads for stability
  }
}

String sendData(String command, const int timeout, boolean debug)
{
  String temp = "";
  Serial1.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial1.available())
    {
      char c = Serial1.read();
      temp += c;
    }
  }
  if (debug)
  {
    Serial.print(temp);
  }
  return temp;
}


void Get_gmap_link(bool makeCall)
{

  {

    Serial.println(lats);
    Serial.println(longi);

    String Gmaps_link = ( "http://maps.google.com/maps?q=" + lats + "+" + longi); //http://maps.google.com/maps?q=38.9419+-78.3020
    //------------------------------------- Sending SMS with Google Maps Link with our Location
    Serial1.println("AT+CMGF=1");
    delay(1000);
    Serial1.println("AT+CMGS=\"" + SOS_NUM + "\"\r");
    delay(1000);

    Serial1.println ("I'm here " + Gmaps_link);
    delay(1000);
    Serial1.println((char)26);
    delay(1000);

    Serial1.println("AT+CMGD=1,4"); // delete stored SMS to save memory
    delay(5000);
  }
  response = "";
  res = "";
  if (makeCall)
  {
    Serial.println("Calling Now");
    Serial1.println("ATD" + SOS_NUM);
  }
}

void MQTT_ReConnect()
{

  String new_msg = "";
  new_msg = sendData("AT+MQTTDISCONN", 2000, DEBUG);
  while ( new_msg.indexOf("OK") == -1 ) {
    new_msg = sendData("AT+MQTTDISCONN", 2000, DEBUG);
    Serial.println("Trying");
  }


  new_msg = "";
  new_msg = sendData("AT+MQTTCONN=\"" + MQTT_BROKER + "\"," + MQTT_PORT + ",\"ABCD\",120,0,\"" + MQTT_USERNAME + "\",\"" + MQTT_PASSWORD + "\"", 3000, DEBUG);
  while ( new_msg.indexOf("OK") == -1 ) {
    new_msg = sendData("AT+MQTTCONN=\"" + MQTT_BROKER + "\"," + MQTT_PORT + ",\"ABCD\",120,0,\"" + MQTT_USERNAME + "\",\"" + MQTT_PASSWORD + "\"", 1000, DEBUG);
    Serial.println("Trying");
  }


}
