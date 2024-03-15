#ifndef AMT22_H
#define AMT22_H

#define AMT22_NOP   0x00
#define AMT22_RESET 0x60
#define AMT22_ZERO  0x70

enum class RES_Type_t {
  RES12, RES14
};

class AMT22 {
  private:
    uint8_t CS;
    uint8_t badReadCounter;
    uint16_t lastGoodRead;
    
    RES_Type_t resType;
    bool encoderHealthy;

    void setCSLine(uint8_t pinState);
    uint8_t spiWriteRead(uint8_t byte, uint8_t releaseLine);
  
  public:
    AMT22(uint8_t CS, RES_Type_t resType);

    bool isHealthy();

    uint16_t getPosition();
    void zeroAMT22();
    void resetAMT22();
};

#endif