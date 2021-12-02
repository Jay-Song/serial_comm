#include "serial_comm_win.h"

using namespace jay;

#define LATENCY_TIME_MSEC (1)

double SerialCommWin::getCurrentTime()
{
  QueryPerformanceCounter(&counter_);
  QueryPerformanceFrequency(&freq_);
  return (double)counter_.QuadPart / (double)freq_.QuadPart * 1000.0;
}

double SerialCommWin::getTimeSinceStart()
{
  double time;

  time = getCurrentTime() - packet_start_time_msec_;
  if (time < 0.0)
  {
    packet_start_time_msec_ = getCurrentTime();
    return 0.0;
  }
  else
    return time;
}


SerialCommWin::SerialCommWin(std::string port_name)
{
  serial_handle_ = INVALID_HANDLE_VALUE;
  baud_rate_ = 57600;
  port_name_ = "\\\\.\\" + port_name;

  packet_start_time_msec_ = 0;
  packet_timeout_msec_ = 0;
  tx_time_per_byte_msec_ = 1.0;
}

SerialCommWin::~SerialCommWin()
{
  closePort();
}

bool SerialCommWin::openPort(const int baud_rate)
{
  if (baud_rate != -1)
    baud_rate_ = baud_rate;
  else
    baud_rate_ = 57600;

  closePort();

  /*
  Function Name - CreateFileA : Open Port
  Arguments:
  LPCSTR lpFileName : Port name
  DWORD dwDesiredAccess : Set as GENERIC_READ | GENERI_WRITE to read and write.
  DWORD dwShareMode : Share mode to usually set as 0 in order to avoid an access by other process
  LPSECURITY_ATTRIBUTES lpSecurityAttributes : Usually set as NULL 
  DWORD dwCreationDisposition : Usually set as OPEN_EXISTING. This is indicating an action when there is no device or file.
  DWORD dwFlagsAndAttributes : Usually set as FILE_ATTRIBUTE_NORMAL
  HANDLE hTemplateFile : Usually set as NULL  */
  serial_handle_ = CreateFileA(port_name_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (serial_handle_ == INVALID_HANDLE_VALUE)
  {
    printf("[SerialComm] Error opening the serial port!\n");
    return false;
  }

  // Configuring Port
  DCB dcb;
  COMMTIMEOUTS timeouts;
  DWORD dwError;

  dcb.DCBlength = sizeof(DCB);
  if (GetCommState(serial_handle_, &dcb) == FALSE) // Get the port state
  {
    printf("[SerialComm] Error getting the port state!\n");
    closePort();
    return false;
  }

  dcb.BaudRate = (DWORD)baud_rate_; // Set baudrate
  dcb.ByteSize = 8;                   // Data bit = 8 bit : usally use 8 bit
  dcb.Parity = NOPARITY;              // parity chek type (even or odd) : usually set as no
  dcb.StopBits = ONESTOPBIT;          // Stop bit = 1 : usually use 1 bit
  dcb.fParity = NOPARITY;             // No Parity check : usually set as no
  dcb.fBinary = 1;                    // Binary mode
  dcb.fNull = 0;                      // Get Null byte. If it is true, Null byte is trown away. 
  dcb.fAbortOnError = 0;              // usually set as 0. If it is true, the comm is terminated when an error occurs.
  dcb.fErrorChar = 0;                 // If it is true and parity check is enabled, the error byte is replaced this char.
  // Not using XOn/XOff
  dcb.fOutX = 0;
  dcb.fInX = 0;
  // Not using H/W flow control
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.fDsrSensitivity = 0;
  dcb.fOutxDsrFlow = 0;
  dcb.fOutxCtsFlow = 0;

  if (SetCommState(serial_handle_, &dcb) == FALSE)
  {
    printf("[SerialComm] Error setting the port dcb!\n");
    closePort();
    return false;
  }
  if (SetupComm(serial_handle_, 4096, 4096) == FALSE) // Buffer size (Rx,Tx)
  {
    printf("[SerialComm] Error setting the port buffer size!\n");
    closePort();
    return false;
  }
  if (PurgeComm(serial_handle_, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == FALSE) // Clear buffer
  {
    printf("[SerialComm] Error purging the port buffer!\n");
    closePort();
    return false;
  }
  if (ClearCommError(serial_handle_, &dwError, NULL) == FALSE)
  {
    printf("[SerialComm] Error cleaning the port error!\n");
    closePort();
    return false;
  }

  // Related to timeout (not use)
  if (GetCommTimeouts(serial_handle_, &timeouts) == FALSE)
  {
    printf("[SerialComm] Error getting the port timeout setting!\n");
    closePort();
    return false;
  }
  // Timeout (Not using timeout)
  // Immediatly return
  timeouts.ReadIntervalTimeout = 0;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 1; // must not be zero.
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 0;
  if (SetCommTimeouts(serial_handle_, &timeouts) == FALSE)
  {
    printf("[SerialComm] Error setting the port timeout!\n");
    closePort();
    return false;
  }

  tx_time_per_byte_msec_ = 100.0 / (double)baud_rate_;
  return true;
}

bool SerialCommWin::closePort()
{
  if (serial_handle_ != INVALID_HANDLE_VALUE)
  {
    CloseHandle(serial_handle_);
    serial_handle_ = INVALID_HANDLE_VALUE;
  }
  return true;
}

bool SerialCommWin::changeBaudRate(const int baud_rate)
{
  if (baud_rate_ != baud_rate)
  {
    closePort();
    baud_rate_ = baud_rate;
    return openPort(baud_rate_);
  }
  else
  {
    printf("[SerialComm] changing the baud rate. The same baud rate is given!\n");
    return true;
  }
}

int SerialCommWin::getBaudRate()
{
  return baud_rate_;
}

int SerialCommWin::readPort(uint8_t *packet, int length)
{
  DWORD dwRead = 0;

  if (ReadFile(serial_handle_, packet, (DWORD)length, &dwRead, NULL) == FALSE)
    return -1;

  return (int)dwRead;
}

int SerialCommWin::writePort(uint8_t *packet, int length)
{
  DWORD dwWrite = 0;

  if (WriteFile(serial_handle_, packet, (DWORD)length, &dwWrite, NULL) == FALSE)
    return -1;

  return (int)dwWrite;
}

void SerialCommWin::setPacketTimeout(uint16_t packet_length)
{
  packet_start_time_msec_ = getCurrentTime();
  packet_timeout_msec_ = (tx_time_per_byte_msec_ * (double)packet_length) + (LATENCY_TIME_MSEC * 2.0) + 2.0;
}

//void SerialCommWin::setPacketTimeout(double wait_time_msec)
//{
//  packet_start_time_msec_ = getCurrentTime();
//  packet_timeout_msec_ = wait_time_msec;
//}

bool SerialCommWin::isPacketTimeout()
{
  if (getTimeSinceStart() > packet_timeout_msec_)
  {
    packet_timeout_msec_ = 0;
    return true;
  }
  else
    return false;
}