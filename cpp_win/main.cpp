#include <iostream>
#include <conio.h>
#include "serial_comm_win.h"



int main(void)
{
  std::cout << "Start" << std::endl;

  jay::SerialCommWin port("COM1");
  port.openPort(1000000); // 1 Mbps

  uint8_t tx_packet[7]; // total_packet_length = 7
  tx_packet[0] = 0xFF;
  tx_packet[1] = 0xFF;
  tx_packet[2] = 0x00; // reserved ID
  tx_packet[3] = 0x03; // length from instruction
  tx_packet[4] = 0x02; // instruction : read
  tx_packet[5] = 0x00; // parameter : 0 for read, 1~255 for write

  // add a checksum to the packet
  uint8_t checksum = 0;
  for (uint16_t idx = 2; idx < 7 - 1; idx++)   // except header, checksum
    checksum += tx_packet[idx];
  tx_packet[6] = ~checksum; // checksum


  uint8_t rx_packet[9];
  rx_packet[0] = 0xFF;
  rx_packet[1] = 0xFF;
  rx_packet[2] = 0x00; // reserved ID
  rx_packet[3] = 0x05; // length from instruction
  rx_packet[4] = 0x00; // parameter 1
  rx_packet[5] = 0x00; // parameter 2
  rx_packet[6] = 0x00; // parameter 3
  rx_packet[7] = 0x00; // parameter 4
  rx_packet[8] = 0x00; // check sum

  while (true)
  {
    if (kbhit())
    {
      if (getch() == 27)
      {
        break;
      }
    }

    // send read instruction
    uint8_t written_packet_length = port.writePort(tx_packet, 7); //total_packet_length = 7
    if (written_packet_length != 7)
    {
      std::cout << "TX_FAIL" << std::endl;
      break;
    }

    // get rx_packet
    uint8_t rx_length = 0;
    uint8_t error = 0;
    checksum = 0;
    port.setPacketTimeout(9);
    while (true)
    {
      rx_length += port.readPort(rx_packet, 9 - rx_length);
      if (rx_length == 9)
      {
        for (uint16_t idx = 2; idx < 9 - 1; idx++)   // except header, checksum
          checksum += rx_packet[idx];
        checksum = ~checksum; // checksum
        if (checksum != rx_packet[8])
        {
          error = 2;
        }
        break;
      }

      if (port.isPacketTimeout())
      {
        error = 1;
        break;
      }
    }
    
    //check error and print the value
    if (error == 0)
    {
      float value = *(float*)&rx_packet[4];
      printf("%x %x %x %x | %f\n", rx_packet[4], rx_packet[5], rx_packet[6], rx_packet[7], value);
    }
    else if (error == 1)
    {
      std::cout << "RX_FAIL" << std::endl;
      break;
    }
    else if (error == 2)
    {
      std::cout << "check sum error" << std::endl;
    }

    Sleep(1000);
  }


  return 0;
}