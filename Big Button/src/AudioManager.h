
#if !defined(AUDIO_MANAGER_H)
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <MD_YX5300.h>
#include <math.h>

#define MP3Stream Serial2
MD_YX5300 mp3(MP3Stream);

class AudioManager
{
public:
    uint8_t folderIndex = 1;
    uint8_t trackIndex = 1;

    AudioManager(){};
    void begin(int8_t rxPin, int8_t txPin);
    void playTrack();
    void playTrack(uint8_t index);
    uint8_t getVolume();
    void setVolume(uint8_t value);
    void loop();

private:
    uint8_t _volume;
};

AudioManager AudioMan;

//------------------------------------------------------------------------------------

uint8_t AudioManager::getVolume()
{
    return _volume;
}

void AudioManager::setVolume(uint8_t value)
{
    _volume = constrain(value, 0, mp3.volumeMax());
    mp3.volume(_volume);
}

void AudioManager::playTrack()
{
    playTrack(trackIndex+1);
}

void AudioManager::playTrack(uint8_t index)
{
    mp3.playSpecific(folderIndex,index);
}

void AudioManager::begin(int8_t rxPin, int8_t txPin)
{
    Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);

    // initialize global libraries
    MP3Stream.begin(MD_YX5300::SERIAL_BPS);
    mp3.begin();
}

void AudioManager::loop()
{
    mp3.check(); // run the mp3 receiver
}

#endif