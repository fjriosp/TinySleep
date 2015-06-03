#ifndef TWISerial_h
#define TWISerial_h

#define TWISERIAL_BUF_SIZE    18              // bytes in message buffer

//class TWI_SERIAL : public Stream
class TWI_SERIAL
{
  private:
	static uint8_t TWISERIAL_Buf[];           // holds I2C send and receive data
	static uint8_t TWISERIAL_BufIdx;          // current number of bytes in the send buff
	
  public:
    TWI_SERIAL();
    void    begin();
    size_t  write(uint8_t);
    uint8_t read(); 
    uint8_t available(); 
    void    flush(void);
};

extern TWI_SERIAL TWISerial;

#endif

