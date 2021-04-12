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
    int heldEnd;
    bool heldEndCheck;

    gameNote()
    {
        entryTime = 0;
        lane = green;
        currentPosY = 0;
        heldTime = 0;
        heldEnd = 0;
        heldEndCheck = false;
    }

    void noHold (int entryTime_)
    {
        entryTime = entryTime_;
        heldTime = entryTime_;
    }
};

struct gameLyrics
{
    std::string lyric;
    Uint32 entryTime;

    gameLyrics(std::string lyric_, Uint32 entryTime_)
    {
        lyric = lyric_;
        entryTime = entryTime_;
    }

    gameLyrics()
    {
        entryTime = 0;
    }
};
const int guitarX = 134;
const int scoreX = 500;
const int scoreY = 300;
textureE playScreenTexture(0, 0);
textureE pauseTexture(0, 0);
textureE guitarTexture(guitarX, 0);
textureE chooseLevelOneTexture(0, 0);
textureE chooseLevelTwoTexture(0, 0);
textureE chooseLevelThreeTexture(0, 0);
textureE gameNoteTexture;
textureE holdNotesTexture;
textureE scoreTexture(scoreX, scoreY);
SDL_Color scoreTextColor = {255, 255, 255};
SDL_Rect noteClips[5];
SDL_Rect holdNoteClips[5];
TTF_Font* scoreFont;

Mix_Music *levelOneSong;

const std::string WINDOW_TITLE = "Keyboard Hero";

//Screen dimension constants
const int SCREEN_WIDTH = 1120;
const int SCREEN_HEIGHT = 630;

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer);

void quitSDL(SDL_Window* &window, SDL_Renderer* &renderer);

void loadMedia(SDL_Renderer* &renderer);

void playLevel(const int &level, bool &isQuit, SDL_Renderer* &renderer);

void pause(bool &isQuit, bool &isLevelEnd, bool &isPause, SDL_Renderer* &renderer, Uint32 &pausedTime);

void loadChart(gameNote (&levelChart)[2000] );

void loadLyrics(gameLyrics (&levelLyrics)[150]);

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
    /*window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP); */
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
        logSDLError(std::cout, "Failed to load font!", true, TTF_Err);
    }
    else
    {
        scoreTexture.loadFromRenderedText("abcdefgh", scoreTextColor, scoreFont, renderer);
        if (scoreTexture.texture == NULL)
        {
            logSDLError(std::cout, "Failed to render score!", false, none);
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
}

void playLevel(const int &level, bool &isQuit, SDL_Renderer* &renderer)
{
    bool isPause = false;
    bool isLevelEnd = false;
    SDL_Event e;
    Uint32 beginningTime = SDL_GetTicks();
    Uint32 passedTime = 0;
    Uint32 pausedTime = 0;
    gameNote levelChart[2000];
    gameLyrics levelLyrics[150];
    gameNote onScreenNotes[50];
    int currentNote = 0;
    int numberOfOnScreenNotes = 0;
    int currentLyric = 0;

    int score = 0;
    loadChart(levelChart);
    loadLyrics(levelLyrics);

    // play music
    if (Mix_PlayingMusic() == 0)
    {
        switch (level)
        {
            case levelChoose1:
                Mix_PlayMusic(levelOneSong, 1);
                break;
        }
    }

    // start level rendering
    while (!isLevelEnd && !isQuit && Mix_PlayingMusic() != 0)
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

            //renderNotesFromMap(renderer, passedTime)
            while (SDL_TICKS_PASSED(passedTime, levelChart[currentNote].entryTime))
            {
                onScreenNotes[numberOfOnScreenNotes] = levelChart[currentNote];
                currentNote++;
                numberOfOnScreenNotes++;
            }

            for (int i = 0; i < numberOfOnScreenNotes; i++)
            {
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
                onScreenNotes[i].currentPosY = (passedTime - onScreenNotes[i].entryTime) / 5 - 99;
                gameNoteTexture.posY = onScreenNotes[i].currentPosY;
                if (onScreenNotes[i].heldTime - onScreenNotes[i].entryTime > 0)
                {
                    if (SDL_TICKS_PASSED(passedTime, onScreenNotes[i].heldTime) && !onScreenNotes[i].heldEndCheck)
                    {
                        onScreenNotes[i].heldEnd = gameNoteTexture.posY;
                        onScreenNotes[i].heldEndCheck = true;
                    }
                    int tmp = 0;
                    if (0 == onScreenNotes[i].heldEnd) tmp = -4;
                    else tmp = gameNoteTexture.posY - onScreenNotes[i].heldEnd;
                    for (int j = gameNoteTexture.posY; j >= tmp; j -= 3)
                    {
                        holdNotesTexture.posY = j;
                        holdNotesTexture.render(renderer, &holdNoteClip);
                    }
                }
                else onScreenNotes[i].heldEnd = onScreenNotes[i].currentPosY;
                gameNoteTexture.render(renderer, &noteClip);
            }

            while (onScreenNotes[0].heldEnd > SCREEN_HEIGHT)
            {
                for (int i = 0; i < numberOfOnScreenNotes; i++)
                {
                    onScreenNotes[i] = onScreenNotes[i + 1];
                }
                numberOfOnScreenNotes--;
            }
            if (SDL_TICKS_PASSED(passedTime, levelLyrics[currentLyric].entryTime))
                {
                    currentLyric++;
                }
            scoreTexture.loadFromRenderedText(levelLyrics[currentLyric - 1].lyric, scoreTextColor, scoreFont, renderer);
            scoreTexture.posX = 500;
            scoreTexture.posY = 100;
            scoreTexture.render(renderer);
            SDL_RenderPresent(renderer);
        }
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
                            int closestNote = -1;
                            for (int i = 0; i < numberOfOnScreenNotes; i++)
                            {
                                if (onScreenNotes[i].lane == green)
                                {
                                    closestNote = i;
                                    break;
                                }
                            }
                            if (closestNote != -1)
                            {
                                if (onScreenNotes[closestNote].currentPosY + 24 <= 594 + 24 &&
                                        onScreenNotes[closestNote].currentPosY + 24 >= 594 - 24)
                                {
                                    for (int i = closestNote; i < numberOfOnScreenNotes; i++)
                                    {
                                        onScreenNotes[i] = onScreenNotes[i + 1];
                                    }
                                    numberOfOnScreenNotes--;
                                    score += 300;
                                }
                            }
                    }
                }
            }
        }
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

void loadChart(gameNote (&levelChart)[2000] )
{
    std::ifstream inFile("assets/LevelOne/Chart.txt");
    int currentNote = 0;
    if (inFile)
    {
        while (!inFile.eof())
        {
            Uint32 entryTime_;
            int lane_;
            Uint32 heldTime_;
            inFile >> entryTime_ >> lane_ >> heldTime_;
            levelChart[currentNote].entryTime = entryTime_;
            levelChart[currentNote].lane = lane_;
            levelChart[currentNote].heldTime = heldTime_;
            currentNote++;
        }
        inFile.close();
        levelChart[currentNote].entryTime = 100000000;
    }
    else
    {
        logSDLError(std::cout, "Could not open chart!", true, none);
    }
}

void loadLyrics(gameLyrics (&levelLyrics)[150])
{
    std::ifstream inFile("assets/LevelOne/Lyrics.txt");
    int currentLyric = 0;
    if (inFile)
    {
        while (!inFile.eof())
        {
            Uint32 entryTime_;
            std::string lyric_;
            inFile >> entryTime_;
            getline(inFile, lyric_);
            levelLyrics[currentLyric].entryTime = entryTime_;
            levelLyrics[currentLyric].lyric = lyric_;
            currentLyric++;
        }
        inFile.close();
        levelLyrics[currentLyric].entryTime = 100000000;
    }
    else
    {
        logSDLError(std::cout, "Could not open lyrics!", false, none);
    }
    //levelLyrics[0].entryTime = 100000000;
}
