

//******************* Pin Configurations *******************//

#define A9G_PON     14
#define A9G_RST    13
#define A9G_LOWP    27
#define SOS_Button 22


void setup() {
  // initialize both serial ports:
  Serial.begin(115200);// For ESP32
  Serial1.begin(115200, SERIAL_8N1, 33, 25); //For A9G

  pinMode(A9G_PON, OUTPUT);//LOW LEVEL ACTIVE
  pinMode(A9G_RST, OUTPUT);//HIGH LEVEL ACTIVE
  pinMode(A9G_LOWP, OUTPUT);//LOW LEVEL ACTIVE
  pinMode(SOS_Button, INPUT);

  digitalWrite(A9G_RST, LOW);
  digitalWrite(A9G_LOWP, HIGH);
  digitalWrite(A9G_PON, HIGH);
  delay(1000);
  digitalWrite(A9G_PON, LOW);
}

void loop()
{
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }
}
