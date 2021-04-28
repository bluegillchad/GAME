#include <iostream>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <string>
#include <SDL_ttf.h>

enum lanes
{
    green,
    red,
    yellow,
    blue,
    orange
};

enum errorType
{
    IMG_Err,
    SDL_Err,
    MIX_Err,
    TTF_Err,
    none
};

enum screenType
{
    levelChoose1,
    levelChoose2,
    levelChoose3,
    playScreen,
    pauseScreen
};

void logSDLError(std::ostream& os, const std::string &msg, bool fatal, int type);

// texture with width and height
struct textureE
{
    SDL_Texture* texture;
    int width;
    int height;

    // position on screen
    int posX;
    int posY;

    textureE()
    {
        texture = NULL;
        width = 0;
        height = 0;
        posX = 0;
        posY = 0;
    }

    textureE(int posX_, int posY_)
    {
        texture = NULL;
        width = 0;
        height = 0;
        posX = posX_;
        posY = posY_;
    }

    void loadTexture(std::string path, SDL_Renderer* &renderer)
    {
        free();
        SDL_Texture* newTexture;
        SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
        if( loadedSurface == NULL ) logSDLError(std::cout, "Unable to load image", true, IMG_Err);
        else
            {
                SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface -> format, 0xFF, 0xFF, 0xFF));
                newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
                if ( newTexture == NULL ) logSDLError(std::cout, "Unable to create texture", true, SDL_Err);
                else
                {
                    width = loadedSurface -> w;
                    height = loadedSurface -> h;
                }
                SDL_FreeSurface(loadedSurface);
            }
        texture = newTexture;
    }

    void loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* &textFont, SDL_Renderer* &renderer)
    {
        free();
        SDL_Surface* textSurface = TTF_RenderText_Solid( textFont, textureText.c_str(), textColor );
        if( textSurface == NULL )
        {
            logSDLError(std::cout, "Unable to render text surface!", true, TTF_Err);
        }
        else
        {
            //Create texture from surface pixels
            texture = SDL_CreateTextureFromSurface( renderer, textSurface );
            if( texture == NULL )
            {
                logSDLError(std::cout, "Unable to create texture from rendered text!", true, SDL_Err );
            }
            else
            {
                //Get image dimensions
                width = textSurface->w;
                height = textSurface->h;
            }

            //Get rid of old surface
            SDL_FreeSurface( textSurface );
        }
    }

    void free()
    {
        if (texture != NULL)
        {
            SDL_DestroyTexture(texture);
            width = 0;
            height = 0;
            posX = 0;
            posY = 0;
        }
    }

    // render at position with rotation and flipping
    void render (SDL_Renderer* &renderer, SDL_Rect* clip = NULL,
                 double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE )
    {
        SDL_Rect renderPos = {posX, posY, width, height};
        if (clip != NULL)
        {
            renderPos.w = clip->w;
            renderPos.h = clip->h;
        }
        SDL_RenderCopyEx(renderer, texture, clip, &renderPos, angle, center, flip);
    }
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

    gameNote()
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
};

struct gameLyrics
{
    std::string lyricOne;
    std::string lyricTwo;
    Uint32 entryTime;

    gameLyrics(std::string lyricOne_, std::string lyricTwo_, Uint32 entryTime_)
    {
        lyricOne = lyricOne_;
        lyricTwo = lyricTwo_;
        entryTime = entryTime_;
    }

    gameLyrics()
    {
        entryTime = 0;
    }
};

const int guitarX = 134;
const int scoreX = 500;
const int scoreY = 270;
const int streakX = 500;
const int streakY = 400;
const int buttonY = 568;
const int multiplierX = 480;
const int multiplierY = 330;
const int hitBox = 68;
textureE playScreenTexture(0, 0);
textureE pauseTexture(0, 0);
textureE guitarTexture(guitarX, 0);
textureE chooseLevelOneTexture(0, 0);
textureE chooseLevelTwoTexture(0, 0);
textureE chooseLevelThreeTexture(0, 0);
textureE gameNoteTexture;
textureE holdNotesTexture;
textureE pressedButtonsTexture(0, buttonY);
textureE textTexture;

const SDL_Color textColor = {255, 255, 255}; // white
SDL_Rect noteClips[5];
SDL_Rect holdNoteClips[5];
SDL_Rect pressedButtonsClips[5];
TTF_Font* scoreFont;
TTF_Font* multiplierFont;

Mix_Music *levelOneSong;

float speed[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};

const std::string WINDOW_TITLE = "Keyboard Hero";

//Screen dimension constants
const int SCREEN_WIDTH = 1120;
const int SCREEN_HEIGHT = 630;

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer);

void quitSDL(SDL_Window* &window, SDL_Renderer* &renderer);

void loadMedia(SDL_Renderer* &renderer);

void playLevel(const int &level, bool &isQuit, SDL_Renderer* &renderer);

void pause(bool &isQuit, bool &isLevelEnd, bool &isPause, SDL_Renderer* &renderer, Uint32 &pausedTime);

void loadChart(gameNote (&levelChart)[2000], Uint32 &musicStart, char* file, int &noteCount, Uint32 &noMultiplierScore);

void loadLyrics(gameLyrics (&levelLyrics)[150], char* file);

void notePressHandle(const int &lane, gameNote (&onScreenNotes)[200], Uint32 &score, int &numberOfOnScreenNotes,
                     const int &keyRepeat, const int &keyState, const Uint32 &passedTime, int &streak, const int &multiplier);

std::string numberToString (const Uint32 &number);

int main(int argc, char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    initSDL(window, renderer);

    loadMedia(renderer);

    bool isQuit = false;
    SDL_Event e;
    bool isChoosingScreen = false;
    int levelPick = levelChoose1;

    while (!isQuit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                isQuit = true;
            }
            else
            {
                if( e.type == SDL_KEYDOWN )
                {
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_ESCAPE:
                            if (isChoosingScreen)
                            {
                                isChoosingScreen = false;
                                levelPick = levelChoose1;
                            }
                            else isQuit = true;
                            break;
                        case SDLK_RETURN:
                            if (isChoosingScreen)
                            {
                                switch (levelPick)
                                {
                                    case levelChoose1:
                                        playLevel(levelChoose1, isQuit, renderer);
                                        break;
                                    case levelChoose2:
                                        playLevel(levelChoose2, isQuit, renderer);
                                        break;
                                    case levelChoose3:
                                        playLevel(levelChoose3, isQuit, renderer);
                                        break;
                                }
                            }
                            else isChoosingScreen = true;
                            break;
                        case SDLK_LEFT:
                            if (isChoosingScreen)
                            {
                                levelPick--;
                                if (levelPick < levelChoose1) levelPick = levelChoose3;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (isChoosingScreen)
                            {
                                levelPick++;
                                if (levelPick > levelChoose3) levelPick = levelChoose1;
                            }
                            break;
                    }
                }
            }
        }
        SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( renderer );

        // picking an image to render
        if (!isChoosingScreen)
        {
            playScreenTexture.render(renderer, NULL);
        }
        else
        {
            switch (levelPick)
            {
            case levelChoose1:
                chooseLevelOneTexture.render(renderer);
                break;
            case levelChoose2:
                chooseLevelTwoTexture.render(renderer);
                break;
            case levelChoose3:
                chooseLevelThreeTexture.render(renderer);
                break;
            }
        }
        //Update screen
        SDL_RenderPresent(renderer);
    }
    playScreenTexture.free();
    pauseTexture.free();
    guitarTexture.free();
    chooseLevelOneTexture.free();
    Mix_FreeMusic(levelOneSong);
    chooseLevelTwoTexture.free();
    chooseLevelThreeTexture.free();
    gameNoteTexture.free();
    TTF_CloseFont(scoreFont);
    quitSDL(window, renderer);
    return 0;
}

void logSDLError(std::ostream& os, const std::string &msg, bool fatal, int type)
{
    switch (type)
    {
        case SDL_Err:
            os << msg << " Error: " << SDL_GetError() << std::endl; break;
        case IMG_Err:
            os << msg << " Error: " << IMG_GetError() << std::endl; break;
        case MIX_Err:
            os << msg << " Error: " << Mix_GetError() << std::endl; break;
        case TTF_Err:
            os << msg << " Error: " << TTF_GetError() << std::endl; break;
        case none:
            break;
    }
    if (fatal) {
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
}

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        logSDLError(std::cout, "SDL_Init", true, SDL_Err);
    }

    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) logSDLError(std::cout, "CreateWindow", true, SDL_Err);
    else
        {
            //Initialize PNG loading
            int imgFlags = IMG_INIT_PNG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) )
            {
                logSDLError(std::cout, "SDL_image could not initialize!", true, IMG_Err);
            }

            //Initialize SDL_mixer
            if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
            {
                logSDLError(std::cout, "SDL_mixer could not initialize!", true, MIX_Err);
            }

            if( TTF_Init() == -1 )
            {
                logSDLError(std::cout, "SDL_ttf could not initialize!", true, TTF_Err );
            }
        }

    //Khi thông thường chạy với môi trường bình thường ở nhà
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC);

    //Khi chạy ở máy thực hành WinXP ở trường (máy ảo)
    //renderer = SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(window));

    if (renderer == NULL) logSDLError(std::cout, "CreateRenderer", true, SDL_Err);
    else
    {
        SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    }

    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    //SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void quitSDL(SDL_Window* &window, SDL_Renderer* &renderer)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	Mix_Quit();
}

void loadMedia(SDL_Renderer* &renderer)
{
    scoreFont = TTF_OpenFont("assets/Raleway-Light.ttf", 28);
    if (scoreFont == NULL)
    {
        logSDLError(std::cout, "Failed to load score font!", true, TTF_Err);
    }
    else
    {
        textTexture.loadFromRenderedText(" ", textColor, scoreFont, renderer);
        if (textTexture.texture == NULL)
        {
            logSDLError(std::cout, "Failed to load textTexture!", true, none);
        }
    }

    playScreenTexture.loadTexture("assets/playScreen.png", renderer);
    if( playScreenTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load playScreen.png!", false, SDL_Err);
    }

    pauseTexture.loadTexture("assets/pause.png", renderer);
    if( pauseTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load pause.png!", false, SDL_Err);
    }

    guitarTexture.loadTexture("assets/guitar.png", renderer);
    if( guitarTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load guitar.png!", false, SDL_Err);
    }

    chooseLevelOneTexture.loadTexture("assets/chooseLevelOne.png", renderer);
    if( chooseLevelOneTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load chooseLevelOne.png!", false, SDL_Err);
    }

    levelOneSong = Mix_LoadMUS("assets/KillTheAche.mp3");
    if( levelOneSong == NULL )
    {
        logSDLError(std::cout, "Failed to load KillTheAche.mp3!", false, MIX_Err);
    }

    chooseLevelTwoTexture.loadTexture("assets/chooseLevelTwo.png", renderer);
    if( chooseLevelTwoTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load chooseLevelTwo.png!", false, SDL_Err);
    }

    chooseLevelThreeTexture.loadTexture("assets/chooseLevelThree.png", renderer);
    if( chooseLevelThreeTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load chooseLevelThree.png!", false, SDL_Err);
    }

    gameNoteTexture.loadTexture("assets/note.png", renderer);
    if( gameNoteTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load note.png!", false, SDL_Err);
    }

    holdNotesTexture.loadTexture("assets/holdNotes.png", renderer);
    if( holdNotesTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load holdNotes.png!", false, SDL_Err);
    }

    pressedButtonsTexture.loadTexture("assets/pressedButton.png", renderer);
    if( pressedButtonsTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load pressedButton.png!", false, SDL_Err);
    }

    noteClips[green].x = 0;
    noteClips[green].y = 0;
    noteClips[green].w = 49;
    noteClips[green].h = 49;

    noteClips[red].x = 49;
    noteClips[red].y = 0;
    noteClips[red].w = 49;
    noteClips[red].h = 49;

    noteClips[blue].x = 49 * 3;
    noteClips[blue].y = 0;
    noteClips[blue].w = 49;
    noteClips[blue].h = 49;

    noteClips[yellow].x = 49 * 2;
    noteClips[yellow].y = 0;
    noteClips[yellow].w = 49;
    noteClips[yellow].h = 49;

    noteClips[orange].x = 49 * 4;
    noteClips[orange].y = 0;
    noteClips[orange].w = 49;
    noteClips[orange].h = 49;

    holdNoteClips[green].x = 0;
    holdNoteClips[green].y = 0;
    holdNoteClips[green].w = 7;
    holdNoteClips[green].h = 7;

    holdNoteClips[red].x = 7;
    holdNoteClips[red].y = 0;
    holdNoteClips[red].w = 7;
    holdNoteClips[red].h = 7;

    holdNoteClips[yellow].x = 7 * 2;
    holdNoteClips[yellow].y = 0;
    holdNoteClips[yellow].w = 7;
    holdNoteClips[yellow].h = 7;

    holdNoteClips[blue].x = 7 * 3;
    holdNoteClips[blue].y = 0;
    holdNoteClips[blue].w = 7;
    holdNoteClips[blue].h = 7;

    holdNoteClips[orange].x = 7 * 4;
    holdNoteClips[orange].y = 0;
    holdNoteClips[orange].w = 7;
    holdNoteClips[orange].h = 7;

    pressedButtonsClips[green].x = 0;
    pressedButtonsClips[green].y = 0;
    pressedButtonsClips[green].w = 53;
    pressedButtonsClips[green].h = 53;

    pressedButtonsClips[red].x = 53;
    pressedButtonsClips[red].y = 0;
    pressedButtonsClips[red].w = 53;
    pressedButtonsClips[red].h = 53;

    pressedButtonsClips[yellow].x = 53 * 2;
    pressedButtonsClips[yellow].y = 0;
    pressedButtonsClips[yellow].w = 53;
    pressedButtonsClips[yellow].h = 53;

    pressedButtonsClips[blue].x = 53 * 3;
    pressedButtonsClips[blue].y = 0;
    pressedButtonsClips[blue].w = 53;
    pressedButtonsClips[blue].h = 53;

    pressedButtonsClips[orange].x = 53 * 4;
    pressedButtonsClips[orange].y = 0;
    pressedButtonsClips[orange].w = 53;
    pressedButtonsClips[orange].h = 53;
}

void playLevel(const int &level, bool &isQuit, SDL_Renderer* &renderer)
{
    bool isPause = false;
    bool isLevelEnd = false;
    bool isPlayingMusic = false;
    bool isSongEnd = false;
    bool isButtonPressed[5];
    bool isStarPower = false;
    SDL_Event e;
    Uint32 beginningTime = SDL_GetTicks();
    Uint32 passedTime = 0;
    Uint32 pausedTime = 0;
    Uint32 musicStart = 0;
    gameNote levelChart[2000];
    gameLyrics levelLyrics[150];
    gameNote onScreenNotes[200];
    int currentNote = 0;
    int numberOfOnScreenNotes = 0;
    int currentLyric = 0;
    int streak = 0;
    int noteCount = 0;
    Uint32 noMultiplierScore = 0;
    int multiplier = 1;
    int notePressedCount = 0;
    double averageMultiplier = 0;

    Uint32 score = 0;
    for (int i = 0; i < 5; i++)
    {
        isButtonPressed[i] = false;
    }
    loadChart(levelChart, musicStart, "assets/LevelOne/Chart.txt", noteCount, noMultiplierScore);
    loadLyrics(levelLyrics, "assets/LevelOne/Lyrics.txt");

    // start level rendering
    while (!isLevelEnd && !isQuit && !isSongEnd)
    {
        if (isPause)
        {
            pause(isQuit, isLevelEnd, isPause, renderer, pausedTime);
        }
        else
        {
            passedTime = SDL_GetTicks() - pausedTime - beginningTime;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF );
            SDL_RenderClear(renderer);
            guitarTexture.render(renderer);

            while (SDL_TICKS_PASSED(passedTime, levelChart[currentNote].entryTime))
            {
                onScreenNotes[numberOfOnScreenNotes] = levelChart[currentNote];
                currentNote++;
                numberOfOnScreenNotes++;
            }

            for (int i = 0; i < numberOfOnScreenNotes; i++)
            {
                // assign texture
                SDL_Rect noteClip, holdNoteClip;
                switch (onScreenNotes[i].lane)
                {
                    case green:
                        gameNoteTexture.posX = 150;
                        holdNotesTexture.posX = 150 + 21;
                        noteClip = noteClips[green];
                        holdNoteClip = holdNoteClips[green];
                        break;
                    case red:
                        gameNoteTexture.posX = 210;
                        holdNotesTexture.posX = 210 + 21;
                        noteClip = noteClips[red];
                        holdNoteClip = holdNoteClips[red];
                        break;
                    case yellow:
                        gameNoteTexture.posX = 270;
                        holdNotesTexture.posX = 270 + 21;
                        noteClip = noteClips[yellow];
                        holdNoteClip = holdNoteClips[yellow];
                        break;
                    case blue:
                        gameNoteTexture.posX = 330;
                        holdNotesTexture.posX = 330 + 21;
                        noteClip = noteClips[blue];
                        holdNoteClip = holdNoteClips[blue];
                        break;
                    case orange:
                        gameNoteTexture.posX = 390;
                        holdNotesTexture.posX = 390 + 21;
                        noteClip = noteClips[orange];
                        holdNoteClip = holdNoteClips[orange];
                        break;
                }

                // posY calculation
                onScreenNotes[i].currentPosY = (passedTime - onScreenNotes[i].entryTime) * speed[1] - 99;
                gameNoteTexture.posY = onScreenNotes[i].currentPosY;

                // render gem
                if (!onScreenNotes[i].isHeld || (onScreenNotes[i].isHeld && !onScreenNotes[i].pressed) )
                {
                    gameNoteTexture.render(renderer, &noteClip);
                }

                // render trail if it is a hold note
                if (onScreenNotes[i].isHeld)
                {
                    if (SDL_TICKS_PASSED(passedTime, onScreenNotes[i].entryTime + onScreenNotes[i].heldTime) && !onScreenNotes[i].heldEndCheck)
                    {
                        onScreenNotes[i].heldLength = gameNoteTexture.posY;
                        onScreenNotes[i].heldEndCheck = true;
                    }
                    int endY;
                    if (0 == onScreenNotes[i].heldLength) endY = -4;
                    else endY = gameNoteTexture.posY - onScreenNotes[i].heldLength - 44;
                    if (!onScreenNotes[i].pressed)
                    {
                        for (int j = gameNoteTexture.posY; j >= endY; j -= 3)
                        {
                            holdNotesTexture.posY = j;
                            holdNotesTexture.render(renderer, &holdNoteClip);
                        }
                    }
                    else
                    {
                        for (int j = 594; j >= endY; j -= 3)
                        {
                            holdNotesTexture.posY = j;
                            holdNotesTexture.render(renderer, &holdNoteClip);
                        }
                    }
                }

                //reset streak if missed note
                if (onScreenNotes[i].currentPosY > 594 && !onScreenNotes[i].pressed) streak = 0;
            }

            while ( (!onScreenNotes[0].isHeld && onScreenNotes[0].currentPosY > SCREEN_HEIGHT) ||
                   (onScreenNotes[0].isHeld && (onScreenNotes[0].currentPosY - onScreenNotes[0].heldLength) > SCREEN_HEIGHT) )
            {
                for (int i = 0; i < numberOfOnScreenNotes; i++)
                {
                    onScreenNotes[i] = onScreenNotes[i + 1];
                }
                numberOfOnScreenNotes--;
            }

            if (SDL_TICKS_PASSED(passedTime, levelLyrics[currentLyric].entryTime + musicStart))
            {
                currentLyric++;
            }

            //render lyric
            if (currentLyric - 1 >= 0)
            {
                textTexture.loadFromRenderedText(levelLyrics[currentLyric - 1].lyricOne, textColor, scoreFont, renderer);
                textTexture.posX = 480;
                textTexture.posY = 100;
                textTexture.render(renderer);
                if (levelLyrics[currentLyric - 1].lyricTwo != " ")
                {
                    textTexture.loadFromRenderedText(levelLyrics[currentLyric - 1].lyricTwo, textColor, scoreFont, renderer);
                    textTexture.posX = 480;
                    textTexture.posY = 150;
                    textTexture.render(renderer);
                }
            }

            // render score
            textTexture.loadFromRenderedText("score: " + numberToString(score), textColor, scoreFont, renderer);
            textTexture.posX = scoreX;
            textTexture.posY = scoreY;
            textTexture.render(renderer);

            // render streak
            textTexture.loadFromRenderedText("streak: " + numberToString(streak), textColor, scoreFont, renderer);
            textTexture.posX = streakX;
            textTexture.posY = streakY;
            textTexture.render(renderer);

            // render multiplier
            textTexture.loadFromRenderedText("x " + numberToString(multiplier), textColor, scoreFont, renderer);
            textTexture.posX = multiplierX;
            textTexture.posY = multiplierY;
            textTexture.render(renderer);

            // light up button if pressed
            for (int i = 0; i < 5; i++)
            {
                if (isButtonPressed[i])
                {
                    SDL_Rect pressedButtonClip;
                    pressedButtonsTexture.posX = 148 + 60 * i;
                    pressedButtonClip = pressedButtonsClips[i];
                    pressedButtonsTexture.render(renderer, &pressedButtonClip);
                }
            }
            SDL_RenderPresent(renderer);
        }
        if (streak <= 10) multiplier = 1;
        else if (streak <= 20) multiplier = 2;
        else if (streak <= 30) multiplier = 3;
        else multiplier = 4;
        if (isStarPower) multiplier *= 2;
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                isQuit = true;
            }
            else
            {
                if( e.type == SDL_KEYDOWN )
                {
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_ESCAPE:
                            isPause = true;
                            break;
                        case SDLK_a:
                            notePressHandle(green, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier);
                            isButtonPressed[green] = true;
                            break;
                        case SDLK_w:
                            notePressHandle(red, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier);
                            isButtonPressed[red] = true;
                            break;
                        case SDLK_e:
                            notePressHandle(yellow, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier);
                            isButtonPressed[yellow] = true;
                            break;
                        case SDLK_r:
                            notePressHandle(blue, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier);
                            isButtonPressed[blue] = true;
                            break;
                        case SDLK_t:
                            notePressHandle(orange, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier);
                            isButtonPressed[orange] = true;
                            break;
                    }
                }
                else if (e.type == SDL_KEYUP)
                {
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_a:
                            notePressHandle(green, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier);
                            isButtonPressed[green] = false;
                            break;
                        case SDLK_w:
                            notePressHandle(red, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier);
                            isButtonPressed[red] = false;
                            break;
                        case SDLK_e:
                            notePressHandle(yellow, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier);
                            isButtonPressed[yellow] = false;
                            break;
                        case SDLK_r:
                            notePressHandle(blue, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier);
                            isButtonPressed[blue] = false;
                            break;
                        case SDLK_t:
                            notePressHandle(orange, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier);
                            isButtonPressed[orange] = false;
                            break;
                    }
                }
            }
        }
        if (!isPlayingMusic && SDL_TICKS_PASSED(passedTime, musicStart))
        {
            switch (level)
            {
                case levelChoose1:
                    Mix_PlayMusic(levelOneSong, 1);
                    Mix_SetMusicPosition(0);
                    break;
            }
            isPlayingMusic = true;
        }

        if (isPlayingMusic && Mix_PlayingMusic() == 0)
        {
            isSongEnd = true;
        }
    }

    if (isSongEnd)
    {

    }
}

void pause(bool &isQuit, bool &isLevelEnd, bool &isPause, SDL_Renderer* &renderer, Uint32 &pausedTime)
{
    Uint32 pauseStart = SDL_GetTicks();
    Mix_PauseMusic();
    SDL_Event e;
    while (isPause)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                isQuit = true;
            }
            else
            {
                if( e.type == SDL_KEYDOWN )
                {
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_ESCAPE:
                            isPause = false;
                            isLevelEnd = true;
                            break;
                        case SDLK_RETURN:
                            isPause = false;
                            break;
                    }
                }
            }
        }
        SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( renderer );
        pauseTexture.render(renderer);
        SDL_RenderPresent(renderer);
    }
    if (!isLevelEnd)
    {
        Mix_ResumeMusic();
        pausedTime += SDL_GetTicks() - pauseStart;
    }
    else Mix_HaltMusic();
}

void loadChart(gameNote (&levelChart)[2000], Uint32 &musicStart, char* file, int &noteCount, Uint32 &noMultiplierScore)
{
    std::ifstream inFile(file);
    int currentNote = 0;
    Uint32 sum = 0;
    if (inFile)
    {
        inFile >> musicStart >> noMultiplierScore;
        while (!inFile.eof())
        {
            Uint32 entryTime_;
            int lane_;
            Uint32 heldTime_;
            inFile >> entryTime_ >> lane_ >> heldTime_;
            levelChart[currentNote].entryTime = entryTime_;
            levelChart[currentNote].lane = lane_;
            levelChart[currentNote].heldTime = heldTime_;
            if (heldTime_ == 0)
            {
                levelChart[currentNote].isHeld = false;
                sum+=50;
            }
            else
            {
                levelChart[currentNote].isHeld = true;
                sum+=heldTime_/10;
            }
            currentNote++;
        }
        inFile.close();
        levelChart[currentNote].entryTime = 100000000;
        noteCount = currentNote;
    }
    else
    {
        logSDLError(std::cout, "Could not open chart!", true, none);
    }
}

void loadLyrics(gameLyrics (&levelLyrics)[150], char* file)
{
    std::ifstream inFile(file);
    int currentLyric = 0;
    if (inFile)
    {
        while (!inFile.eof())
        {
            Uint32 entryTime_;
            std::string lyric_;
            int numberOfLines;
            inFile >> entryTime_ >> numberOfLines;
            if (numberOfLines == 1) levelLyrics[currentLyric].lyricTwo = " ";
            for (int i = 1; i <= numberOfLines; i++)
            {
                getline(inFile, lyric_);
                if (i == 1)
                {
                    levelLyrics[currentLyric].lyricOne = lyric_;
                }
                if (i == 2)
                {
                    levelLyrics[currentLyric].lyricTwo = lyric_;
                }
            }
            levelLyrics[currentLyric].entryTime = entryTime_;
            currentLyric++;
        }
        inFile.close();
        levelLyrics[currentLyric].entryTime = 100000000;
        levelLyrics[currentLyric].lyricOne = "default text";
        levelLyrics[currentLyric].lyricTwo = "default text";
    }
    else
    {
        logSDLError(std::cout, "Could not open lyrics!", false, none);
    }
}

void notePressHandle(const int &lane, gameNote (&onScreenNotes)[200], Uint32 &score, int &numberOfOnScreenNotes,
                     const int &keyRepeat, const int &keyState, const Uint32 &passedTime, int &streak, const int &multiplier)
{
    int closestNote = -1;
    for (int i = 0; i < numberOfOnScreenNotes; i++)
    {
        if (onScreenNotes[i].lane == lane)
        {
            closestNote = i;
            break;
        }
    }
    if (closestNote != -1)
    {
        if (keyState == SDL_KEYDOWN)
        {
            if (keyRepeat == 0)
            {
                if (onScreenNotes[closestNote].currentPosY <= 594 &&
                    onScreenNotes[closestNote].currentPosY >= 594 - hitBox)
                {
                    onScreenNotes[closestNote].pressed = true;
                    if (!onScreenNotes[closestNote].isHeld)
                    {
                        for (int i = closestNote; i < numberOfOnScreenNotes; i++)
                        {
                            onScreenNotes[i] = onScreenNotes[i + 1];
                        }
                        numberOfOnScreenNotes--;
                        score += 50 * multiplier;
                    }
                    else
                    {
                        onScreenNotes[closestNote].heldStartTime = passedTime;
                    }
                    streak++;
                }
                else streak = 0;
            }
        }
        else if (keyState == SDL_KEYUP && onScreenNotes[closestNote].pressed && !onScreenNotes[closestNote].released)
        {
            if (passedTime - onScreenNotes[closestNote].heldStartTime > onScreenNotes[closestNote].heldTime)
            {
                score += onScreenNotes[closestNote].heldTime / 10 * multiplier;
            }
            else
            {
                score += ( passedTime - onScreenNotes[closestNote].heldStartTime ) / 10 * multiplier;
            }
            onScreenNotes[closestNote].released = true;
        }
    }
    else if (keyState == SDL_KEYDOWN)
    {
        streak = 0;
    }
}

std::string numberToString (const Uint32 &number)
{
    std::string s;
    if (number == 0) return "0";
    int number1 = number;
    while (number1 != 0)
    {
        int tmp = number1 % 10;
        s = char (tmp + 48) + s;
        number1 /= 10;
    }
    return s;
}
