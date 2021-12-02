#ifndef SERIAL_COMM_WIN_H_
#define SERIAL_COMM_WIN_H_

#include <Windows.h>
#include <string>

namespace jay
{
  class SerialCommWin
  {
  private:
    HANDLE serial_handle_;
    LARGE_INTEGER freq_, counter_;
    
    int baud_rate_;
    std::string port_name_;

    double packet_start_time_msec_;
    double packet_timeout_msec_;
    double tx_time_per_byte_msec_;

    double getCurrentTime();
    double getTimeSinceStart();

  public:
    SerialCommWin(std::string port_name);
    virtual ~SerialCommWin();

    bool openPort(const int baud_rate = -1);
    bool closePort();
    
    bool changeBaudRate(const int baud_rate);
    int getBaudRate();

    int readPort(uint8_t *packet, int length);
    int writePort(uint8_t *packet, int length);

    void setPacketTimeout(uint16_t packet_length);
//    void setPacketTimeout(double wait_time_msec);

    bool isPacketTimeout();
  };
}


#endif

