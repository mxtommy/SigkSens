#ifndef _i2c_H_
#define _i2c_H_

class I2cBus {
  private:
    bool started;
    bool scanAddress(uint8_t address);

  public:
    I2cBus();
    void start();
    void scanAll();

};


  
#endif
