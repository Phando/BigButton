#if !defined(LED_MANAGER_H)
#define LED_MANAGER_H

#include <Arduino.h>
#include <ESPUtils.h>
#include <FastLED.h>

#define LED_TYPE WS2811
#define COLOR_ORDER GRB // BRG/RGB/GRB
#define BUTTON_COUNT 12
#define BUTTON_PIN 12
#define BODY_COUNT 24
//#define DATA_PIN 27
#define BODY_PIN 27
#define BRIGHTNESS 255

CRGB buttonRing[BUTTON_COUNT];
CRGB bodyRing[BODY_COUNT];
CRGB sfBlue = CRGB( 23,  160,  219);
uint8_t gHue = 0;

class LEDManager
{
public:
    bool available = false;
    int brightness = 255;
    int frameRate = 30;
    int patternIndex = 1;
    UtilMessageCallback callback;

    LEDManager(){};
    void begin();
    void loop();

    void clearRing(CRGB ring[], int ringCount, CRGB color);
    void idlePattern();
    void solidPattern(bool fillTop=false);
    void subtlePattern(bool fillTop=false);
    void cyclePattern(bool fillTop=false);
    void flashPattern();
    void updatePattern();
    void setColor(String color);

private:
    bool animating = false;
    int getPIndex(int offset, int count);

    //    void sendCommand(String command);
    //    void sendCommand(int command);
};

LEDManager LEDMan;

//------------------------------------------------------------------------------------
int LEDManager::getPIndex(int offset, int count){
    while( offset >= count){
        offset -= count;
    }
    return offset;
}

void LEDManager::clearRing(CRGB ring[], int ringCount, CRGB color = CRGB::Black)
{
    for (int i = 0; i < ringCount; i++)
    {
        ring[i] = color;
    }
}

void addGlitter(CRGB ring[], int ringCount, double percent){
    int count = ringCount * percent;
    
    for(int i=0; i<count; i++){
        ring[random(0, ringCount-1)] = CRGB::White;
    }
}

void LEDManager::idlePattern()
{
    FastLED.setBrightness(0);
    clearRing(buttonRing, BUTTON_COUNT, CRGB::Black);
    clearRing(bodyRing, BODY_COUNT, CRGB::Black);
}

void LEDManager::solidPattern(bool fillTop)
{
    FastLED.setBrightness(brightness);
    CRGB topFill = fillTop ? sfBlue : CRGB::Black;
    clearRing(buttonRing, BUTTON_COUNT, topFill);
    clearRing(bodyRing, BODY_COUNT, sfBlue);
}

void LEDManager::subtlePattern(bool fillTop)
{
    solidPattern(fillTop);
    addGlitter(bodyRing, BODY_COUNT, 0.05);
}

void LEDManager::cyclePattern(bool fillTop)
{
    FastLED.setBrightness(brightness);
    if( fillTop ){
        fill_rainbow( buttonRing, BUTTON_COUNT, gHue, 1);
    }
    else {
        clearRing(buttonRing, BUTTON_COUNT);
    }
    
    fill_rainbow( bodyRing, BODY_COUNT, gHue, 1);
}

void LEDManager::flashPattern()
{
    FastLED.setBrightness(255);
    clearRing(buttonRing, BUTTON_COUNT, CRGB::White);
    clearRing(bodyRing, BODY_COUNT, CRGB::White);
    FastLED.show();
}

void LEDManager::updatePattern()
{
    switch (patternIndex)
    {
    case 1:
        solidPattern();
        break;
    case 2:
        solidPattern(true);
        break;
    case 3:
        subtlePattern();
        break;
    case 4:
        subtlePattern(true);
        break;
    case 5:
        cyclePattern();
        break;
    case 6:
        cyclePattern(true);
        break;
    default:
        idlePattern();
        break;
    }
}

void LEDManager::begin()
{
    Serial.println("LED Manager Begin...");
    FastLED.addLeds<LED_TYPE, BUTTON_PIN, COLOR_ORDER>(buttonRing, BUTTON_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, BODY_PIN, COLOR_ORDER>(bodyRing, BODY_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(brightness);

    clearRing(buttonRing, BUTTON_COUNT);
    clearRing(bodyRing, BODY_COUNT);
}

//------------------------------------------------------------------------------------
void LEDManager::loop()
{
    if (animating)
    {
        return;
    }

    EVERY_N_MILLISECONDS(frameRate) { 
        gHue++; 
        updatePattern();
        FastLED.show();
    }
}

#endif