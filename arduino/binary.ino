float value = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);
  while (!Serial)
  {
    delay(1);
  }
}

int idx = 0;
unsigned char rx_packet[7];

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
      rx_packet[idx] = Serial.read();
      Serial.write(rx_packet[idx]);
      idx++;
  }

  if (idx == 7)
  {
    int i = 0;
    for (i = 0; i < 6; i++)
    {
      if (rx_packet[i] == 0xFF) &&( rx_packet[i+1] == 0xFF)
      {
        break;
      }
    }

    if(i != 0)
    
  }

}
