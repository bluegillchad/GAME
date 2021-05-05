#ifndef GAME__H
#define GAME__H

const std::string WINDOW_TITLE = "Keyboard Hero";

//Screen dimension constants
const int SCREEN_WIDTH = 1120;
const int SCREEN_HEIGHT = 630;

const int hitBox = 68;

float noteSpeed[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};
float starMultiplier[7] = {0.5, 1, 1.5, 2, 2.5, 3, 3.5};

enum lanes
{
    green,
    red,
    yellow,
    blue,
    orange
};

struct gameNote
{
    Uint32 entryTime;
    int lane;
    int currentPosY;
    Uint32 heldTime;
    Uint32 heldStartTime;
    int heldLength;
    bool heldEndCheck;
    bool isHeld;
    bool pressed;
    bool released;

    gameNote();
};

struct gameLyrics
{
    std::string lyricOne;
    std::string lyricTwo;
    Uint32 entryTime;

    gameLyrics(std::string lyricOne_, std::string lyricTwo_, Uint32 entryTime_);

    gameLyrics();
};

gameNote::gameNote()
{
    entryTime = 0;
    lane = green;
    currentPosY = 0;
    heldTime = 0;
    heldStartTime = 0;
    heldLength = 0;
    heldEndCheck = false;
    isHeld = false;
    pressed = false;
    released = false;
}

gameLyrics::gameLyrics(std::string lyricOne_, std::string lyricTwo_, Uint32 entryTime_)
{
    lyricOne = lyricOne_;
    lyricTwo = lyricTwo_;
    entryTime = entryTime_;
}

gameLyrics::gameLyrics()
{
    entryTime = 0;
}

#endif // GAME__H
