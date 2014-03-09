#ifndef ethernetclient_h
#define ethernetclient_h
#include <avr/pgmspace.h>
#include "Arduino.h"	
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"

class EthernetClient : public Client {

public:
  EthernetClient();
  EthernetClient(uint8_t sock);

  uint8_t status();
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  /* Xavier's Custom methods */
  virtual size_t pushTx(char);
  virtual size_t pushTx(char []);
//  size_t pushTx(const uint8_t *, size_t);
  size_t pushTx(uint8_t *, size_t);
  virtual size_t pushTx(const __FlashStringHelper* buf);
  virtual size_t flushTx();
  uint8_t getSocket();

  friend class EthernetServer;
  
  using Print::write;

private:
  uint16_t data_offset;
  static uint16_t _srcport;
  uint8_t _sock;
};

#endif
