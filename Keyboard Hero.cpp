#include <iostream>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <string>
#include <SDL_ttf.h>
#include "SDLstuff.h"
#include "game.h"
#include "otherstuff.h"

enum screenType
{
    levelChoose1,
    levelChoose2,
    levelChoose3,
    playScreen,
    pauseScreen
};

const int buttonY = 568;
textureE playScreenTexture(0, 0);
textureE pauseTexture(0, 0);
textureE guitarTexture(134, 0);
textureE levelOneAlbum(770, 50);
textureE levelTwoAlbum(770, 50);
textureE levelThreeAlbum(770, 50);
textureE backgroundTexture(0, 0);
textureE bigBlackRectangleTexture(0, 0);
textureE bigBlackRectangle2Texture(0, 0);
textureE gameNoteTexture;
textureE holdNotesTexture;
textureE pressedButtonsTexture(0, buttonY);
textureE textTexture;
textureE comingSoonTexture(0, 0);
textureE scoreAndStarTexture(620, 410);

const SDL_Color textColor = {255, 255, 255}; // white
SDL_Rect noteClips[5];
SDL_Rect holdNoteClips[5];
SDL_Rect pressedButtonsClips[5];
TTF_Font* RalewayLightFont;

Mix_Music *levelOneSong;
Mix_Music *levelTwoSong;
Mix_Music *levelThreeSong;

void loadMedia(SDL_Renderer* &renderer);

void playLevel(const int &level, bool &isQuit, SDL_Renderer* &renderer);

void pause(bool &isQuit, bool &isLevelEnd, bool &isPause, SDL_Renderer* &renderer, Uint32 &pausedTime);

void loadChart(gameNote (&levelChart)[2000], Uint32 &musicStart, char* file, int &noteCount, Uint32 &noMultiplierScore, int &speed);

void loadLyrics(gameLyrics (&levelLyrics)[150], char* file);

void notePressHandle(const int &lane, gameNote (&onScreenNotes)[200], Uint32 &score, int &numberOfOnScreenNotes,
                     const int &keyRepeat, const int &keyState, const Uint32 &passedTime, int &streak, const int &multiplier, int &accuracy);

void getHighScore(int (&highStar)[10], int (&highAccuracy)[10], Uint32 (&highScore)[10], char* file);

void setHighScore(int &highStar, int &highAccuracy, Uint32 &highScore, char* file);

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
    Uint32 startMusicLoopTime = 0;

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
                                Mix_HaltMusic();
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
                                Mix_HaltMusic();
                                levelPick--;
                                if (levelPick < levelChoose1) levelPick = levelChoose3;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (isChoosingScreen)
                            {
                                Mix_HaltMusic();
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
            std::string songTitle;
            std::string artist;
            std::string genre;
            std::string releaseYear;
            std::string songLength;
            bool isLyricsAvailble;
            char* highscoreFilePath;
            backgroundTexture.render(renderer);
            bigBlackRectangleTexture.render(renderer);
            switch (levelPick)
            {
                case levelChoose1:
                    songTitle = "Kill the Ache";
                    artist = "Currents";
                    genre = "Metalcore";
                    releaseYear = "2020";
                    songLength  = "03:53";
                    isLyricsAvailble = true;
                    highscoreFilePath = "assets/LevelOne/Highscore.txt";
                    levelOneAlbum.render(renderer);
                    if (Mix_PlayingMusic() == 0)
                    {
                        startMusicLoopTime = SDL_GetTicks();
                        Mix_FadeInMusicPos(levelOneSong, 1, 1000, 74.7);
                    }
                    if (SDL_TICKS_PASSED(SDL_GetTicks() - startMusicLoopTime, (103 - 74.7) * 1000))
                    {
                        Mix_FadeOutMusic(1000);
                    }
                    break;
                case levelChoose2:
                    songTitle = "Me, Myself and Hyde";
                    artist = "Ice Nine Kills";
                    genre = "Metalcore";
                    releaseYear = "2015";
                    songLength  = "04:04";
                    isLyricsAvailble = false;
                    levelTwoAlbum.render(renderer);
                    highscoreFilePath = "assets/LevelTwo/Highscore.txt";
                    if (Mix_PlayingMusic() == 0)
                    {
                        startMusicLoopTime = SDL_GetTicks();
                        Mix_FadeInMusicPos(levelTwoSong, 1, 1000, 82);
                    }
                    if (SDL_TICKS_PASSED(SDL_GetTicks() - startMusicLoopTime, (105.5 - 82) * 1000))
                    {
                        Mix_FadeOutMusic(1000);
                    }
                    break;
                case levelChoose3:
                    songTitle = "Gone With The Wind";
                    artist = "Architects";
                    genre = "Metalcore";
                    releaseYear = "2016";
                    songLength  = "03:49";
                    isLyricsAvailble = true;
                    levelThreeAlbum.render(renderer);
                    highscoreFilePath = "assets/LevelThree/Highscore.txt";
                    if (Mix_PlayingMusic() == 0)
                    {
                        startMusicLoopTime = SDL_GetTicks();
                        Mix_FadeInMusicPos(levelThreeSong, 1, 1000, 71);
                    }
                    if (SDL_TICKS_PASSED(SDL_GetTicks() - startMusicLoopTime, (90 - 71) * 1000))
                    {
                        Mix_FadeOutMusic(1000);
                    }
                    break;
            }
            changeFontSize(RalewayLightFont, 40, "assets/Raleway-Light.ttf");
            renderText("Highscores", textColor, RalewayLightFont, renderer, textTexture, 300, 50);
            changeFontSize(RalewayLightFont, 28, "assets/Raleway-Light.ttf");
            renderText(songTitle, textColor, RalewayLightFont, renderer, textTexture, 775, 362);
            changeFontSize(RalewayLightFont, 20, "assets/Raleway-Light.ttf");
            renderText(artist, textColor, RalewayLightFont, renderer, textTexture, 785, 420);
            renderText(genre, textColor, RalewayLightFont, renderer, textTexture, 785, 480);
            renderText(releaseYear, textColor, RalewayLightFont, renderer, textTexture, 785, 540);
            renderText(songLength, textColor, RalewayLightFont, renderer, textTexture, 935, 420);
            if (isLyricsAvailble) renderText("Lyrics: Yes", textColor, RalewayLightFont, renderer, textTexture, 935, 480);
            else renderText("Lyrics: No", textColor, RalewayLightFont, renderer, textTexture, 935, 480);

            int highStar[10];
            int highAccuracy[10];
            Uint32 highScore[10];
            getHighScore(highStar, highAccuracy, highScore, highscoreFilePath);
            for (int i = 0; i < 10; i++)
            {
                renderText(numberToString(i+1) + ".", textColor, RalewayLightFont, renderer, textTexture, 70, 120 + i * 47);
                renderText(numberToString(highStar[i]) + " Stars", textColor, RalewayLightFont, renderer, textTexture, 100, 120 + i * 47);
                renderText(numberToString(highAccuracy[i]) + '%', textColor, RalewayLightFont, renderer, textTexture, 200, 120 + i * 47);
                renderText(numberToString(highScore[i]), textColor, RalewayLightFont, renderer, textTexture, 300, 120 + i * 47);
            }
        }
        //Update screen
        SDL_RenderPresent(renderer);
    }
    playScreenTexture.free();
    backgroundTexture.free();
    pauseTexture.free();
    guitarTexture.free();
    levelOneAlbum.free();
    levelTwoAlbum.free();
    levelThreeAlbum.free();
    Mix_FreeMusic(levelOneSong);
    Mix_FreeMusic(levelTwoSong);
    Mix_FreeMusic(levelThreeSong);
    gameNoteTexture.free();
    holdNotesTexture.free();
    pressedButtonsTexture.free();
    textTexture.free();
    TTF_CloseFont(RalewayLightFont);
    quitSDL(window, renderer);
    return 0;
}

void loadMedia(SDL_Renderer* &renderer)
{
    RalewayLightFont = TTF_OpenFont("assets/Raleway-Light.ttf", 28);
    if (RalewayLightFont == NULL)
    {
        logSDLError(std::cout, "Failed to load Raleway-Light.ttf!", true, TTF_Err);
    }
    else
    {
        textTexture.loadFromRenderedText(" ", textColor, RalewayLightFont, renderer);
        if (textTexture.texture == NULL)
        {
            logSDLError(std::cout, "Failed to load textTexture!", true, none);
        }
    }

    scoreAndStarTexture.loadTexture("assets/scoreAndStar.png", renderer, true);
    if (scoreAndStarTexture.texture == NULL)
    {
        logSDLError(std::cout, "Failed to load scoreAndStar.png!", false, SDL_Err);
    }

    comingSoonTexture.loadTexture("assets/comingSoon.png", renderer, false);
    if (comingSoonTexture.texture == NULL)
    {
        logSDLError(std::cout, "Failed to load comingSoon.png!", false, SDL_Err);
    }

    bigBlackRectangleTexture.loadTexture("assets/bigBlackRectangle.png", renderer, true);
    if (bigBlackRectangleTexture.texture == NULL)
    {
        logSDLError(std::cout, "Failed to load bigBlackRectangle.png!", false, SDL_Err);
    }
    else
    {
        SDL_SetTextureBlendMode(bigBlackRectangleTexture.texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(bigBlackRectangleTexture.texture, 100);
    }

    bigBlackRectangle2Texture.loadTexture("assets/bigBlackRectangle2.png", renderer, true);
    if (bigBlackRectangle2Texture.texture == NULL)
    {
        logSDLError(std::cout, "Failed to load bigBlackRectangle2.png!", false, SDL_Err);
    }
    else
    {
        SDL_SetTextureBlendMode(bigBlackRectangle2Texture.texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(bigBlackRectangle2Texture.texture, 100);
    }

    backgroundTexture.loadTexture("assets/background.png", renderer, false);
    if (backgroundTexture.texture == NULL)
    {
        logSDLError(std::cout, "Failed to load background.png!", false, SDL_Err);
    }

    playScreenTexture.loadTexture("assets/playScreen.png", renderer, false);
    if( playScreenTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load playScreen.png!", false, SDL_Err);
    }

    pauseTexture.loadTexture("assets/pause.png", renderer, false);
    if( pauseTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load pause.png!", false, SDL_Err);
    }

    guitarTexture.loadTexture("assets/guitar.png", renderer, true);
    if( guitarTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load guitar.png!", false, SDL_Err);
    }

    levelOneAlbum.loadTexture("assets/LevelOne/album.png", renderer, false);
    if( levelOneAlbum.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load LevelOne/album.png!", false, SDL_Err);
    }

    levelOneSong = Mix_LoadMUS("assets/LevelOne/song.mp3");
    if( levelOneSong == NULL )
    {
        logSDLError(std::cout, "Failed to load LevelOne/song.mp3!", false, MIX_Err);
    }

    levelTwoAlbum.loadTexture("assets/LevelTwo/album.png", renderer, false);
    if( levelTwoAlbum.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load LevelTwo/album.png!", false, SDL_Err);
    }

    levelTwoSong = Mix_LoadMUS("assets/LevelTwo/song.mp3");
    if( levelTwoSong == NULL )
    {
        logSDLError(std::cout, "Failed to load LevelTwo/song.mp3!", false, MIX_Err);
    }

    levelThreeAlbum.loadTexture("assets/LevelThree/album.png", renderer, false);
    if( levelThreeAlbum.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load LevelThree/album.png!", false, SDL_Err);
    }

    levelThreeSong = Mix_LoadMUS("assets/LevelThree/song.mp3");
    if( levelThreeSong == NULL )
    {
        logSDLError(std::cout, "Failed to load LevelThree/song.mp3!", false, MIX_Err);
    }

    gameNoteTexture.loadTexture("assets/note.png", renderer, true);
    if( gameNoteTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load note.png!", false, SDL_Err);
    }

    holdNotesTexture.loadTexture("assets/holdNotes.png", renderer, true);
    if( holdNotesTexture.texture == NULL )
    {
        logSDLError(std::cout, "Failed to load holdNotes.png!", false, SDL_Err);
    }

    pressedButtonsTexture.loadTexture("assets/pressedButton.png", renderer, true);
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
    if (level == levelChoose2)
    {
        Mix_HaltMusic();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF );
        SDL_RenderClear(renderer);
        comingSoonTexture.render(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(5000);
        return;
    }
    changeFontSize(RalewayLightFont, 28, "assets/Raleway-Light.ttf");
    bool isPause = false;
    bool isLevelEnd = false;
    bool isPlayingMusic = false;
    bool isSongEnd = false;
    bool isButtonPressed[5];
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
    int accuracy = 0;
    int highestStreak = 0;
    Uint32 noMultiplierScore = 0;
    int multiplier = 1;
    int notePressedCount = 0;
    int star = 0;
    int speed = 0;

    Uint32 score = 0;
    for (int i = 0; i < 5; i++)
    {
        isButtonPressed[i] = false;
    }

    char* chartPath;
    char* lyricsPath;
    switch (level)
    {
        case levelChoose1:
            chartPath = "assets/LevelOne/Chart.txt";
            lyricsPath = "assets/LevelOne/Lyrics.txt";
            break;
        case levelChoose3:
            chartPath = "assets/LevelThree/Chart.txt";
            lyricsPath = "assets/LevelThree/Lyrics.txt";
            break;
    }
    loadChart(levelChart, musicStart, chartPath, noteCount, noMultiplierScore, speed);
    loadLyrics(levelLyrics, lyricsPath);

    Mix_HaltMusic();
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
            scoreAndStarTexture.render(renderer);

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
                onScreenNotes[i].currentPosY = (passedTime - onScreenNotes[i].entryTime) * noteSpeed[speed] - 99;
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
                renderText(levelLyrics[currentLyric - 1].lyricOne, textColor, RalewayLightFont, renderer, textTexture, 480, 100);
                if (levelLyrics[currentLyric - 1].lyricTwo != " ")
                {
                    renderText(levelLyrics[currentLyric - 1].lyricTwo, textColor, RalewayLightFont, renderer, textTexture, 480, 150);
                }
            }

            // render score
            renderText(numberToString(score), textColor, RalewayLightFont, renderer, textTexture, 642, 435);
            // render streak
            renderText(numberToString(streak), textColor, RalewayLightFont, renderer, textTexture, 715, 492);
            // render multiplier
            renderText("x " + numberToString(multiplier), textColor, RalewayLightFont, renderer, textTexture, 480, 330);
            // render star
            renderText(numberToString(star), textColor, RalewayLightFont, renderer, textTexture, 907, 441);

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
        if (score >= noMultiplierScore * starMultiplier[star]) star++;
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
                            notePressHandle(green, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[green] = true;
                            break;
                        case SDLK_w:
                            notePressHandle(red, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[red] = true;
                            break;
                        case SDLK_e:
                            notePressHandle(yellow, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[yellow] = true;
                            break;
                        case SDLK_r:
                            notePressHandle(blue, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[blue] = true;
                            break;
                        case SDLK_t:
                            notePressHandle(orange, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYDOWN, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[orange] = true;
                            break;
                    }
                }
                else if (e.type == SDL_KEYUP)
                {
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_a:
                            notePressHandle(green, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[green] = false;
                            break;
                        case SDLK_w:
                            notePressHandle(red, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[red] = false;
                            break;
                        case SDLK_e:
                            notePressHandle(yellow, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[yellow] = false;
                            break;
                        case SDLK_r:
                            notePressHandle(blue, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[blue] = false;
                            break;
                        case SDLK_t:
                            notePressHandle(orange, onScreenNotes, score, numberOfOnScreenNotes, e.key.repeat, SDL_KEYUP, passedTime, streak, multiplier, accuracy);
                            isButtonPressed[orange] = false;
                            break;
                    }
                }
            }
        }
        if (streak > highestStreak) highestStreak = streak;
        if (!isPlayingMusic && SDL_TICKS_PASSED(passedTime, musicStart))
        {
            switch (level)
            {
                case levelChoose1:
                    Mix_PlayMusic(levelOneSong, 1);
                    break;
                case levelChoose3:
                    Mix_PlayMusic(levelThreeSong, 1);
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
        char *highscoreFilePath;
        std::string scoreDisplay = "Score: " + numberToString(score);
        switch (level)
        {
            case levelChoose1:
                highscoreFilePath = "assets/LevelOne/Highscore.txt";
                break;
            case levelChoose2:
                highscoreFilePath = "assets/LevelTwo/Highscore.txt";
                break;
            case levelChoose3:
                highscoreFilePath = "assets/LevelThree/Highscore.txt";
                break;
        }
        int highStar[10];
        int highAccuracy[10];
        Uint32 highScore[10];
        int accuracyPercent = double(accuracy)/noteCount * 100;
        getHighScore(highStar, highAccuracy, highScore, highscoreFilePath);
        if (score > highScore[0])
        {
            scoreDisplay += "   New high score!";
        }
        setHighScore(star, accuracyPercent, score, highscoreFilePath);
        while (!isQuit && !isLevelEnd)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF );
            SDL_RenderClear(renderer);
            backgroundTexture.render(renderer);
            bigBlackRectangle2Texture.render(renderer);
            renderText(scoreDisplay, textColor, RalewayLightFont, renderer, textTexture, 70, 70);
            renderText("Stars: " + numberToString(star), textColor, RalewayLightFont, renderer, textTexture, 70, 120);
            renderText("Accuracy: " + numberToString(accuracy) + '/' + numberToString(noteCount) + " (" +
                        numberToString(accuracyPercent) + "%)", textColor, RalewayLightFont, renderer, textTexture, 70, 170);
            renderText("Highest streak: " + numberToString(highestStreak), textColor, RalewayLightFont, renderer, textTexture, 70, 220);
            if (accuracy == noteCount)
            {
                renderText("Full combo!" + numberToString(highestStreak), textColor, RalewayLightFont, renderer, textTexture, 70, 270);
            }
            SDL_RenderPresent(renderer);
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
                                isLevelEnd = true;
                                break;
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

void loadChart(gameNote (&levelChart)[2000], Uint32 &musicStart, char* file, int &noteCount, Uint32 &noMultiplierScore, int &speed)
{
    std::ifstream inFile(file);
    int currentNote = 0;
    Uint32 sum = 0;
    if (inFile)
    {
        inFile >> speed >> noMultiplierScore;
        musicStart = (594 - hitBox + 99) / noteSpeed[speed];
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
                     const int &keyRepeat, const int &keyState, const Uint32 &passedTime, int &streak, const int &multiplier, int &accuracy)
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
                    accuracy++;
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
    else if (keyState == SDL_KEYDOWN && keyRepeat == 0)
    {
        streak = 0;
    }
}

void getHighScore(int (&highStar)[10], int (&highAccuracy)[10], Uint32 (&highScore)[10], char* file)
{
    std::ifstream inFile(file);
    if (inFile)
    {
        for (int i = 0; i < 10; i++) inFile >> highStar[i] >> highAccuracy[i] >> highScore[i];
    }
    else
    {
        logSDLError(std::cout, "Could not load Highscore.txt!", true, none);
    }
}

void setHighScore(int &highStar, int &highAccuracy, Uint32 &highScore, char* file)
{
    int star[10];
    int accuracy[10];
    Uint32 score[10];
    int place = 10;
    std::ifstream inFile(file);
    if (inFile)
    {
        for (int i = 0; i < 10; i++)
        {
            inFile >> star[i] >> accuracy[i] >> score[i];
        }
    }
    else
    {
        logSDLError(std::cout, "Could not load Highscore.txt!", true, none);
    }
    for (int i = 9; i >= 0; i--) if (highScore > score[i]) place--;
    if (place != 10)
    {
        for (int i = 9; i > place; i--)
        {
            star[i] = star[i - 1];
            accuracy[i] = accuracy[i - 1];
            score[i] = score[i - 1];
        }
        star[place] = highStar;
        accuracy[place] = highAccuracy;
        score[place] = highScore;
        std::ofstream outFile(file);
        if (outFile)
        {
            for (int i = 0; i < 10; i++) outFile << star[i] << ' ' << accuracy[i] << ' ' << score[i] << std::endl;
        }
        else
        {
            logSDLError(std::cout, "Could not load Highscore.txt!", true, none);
        }
    }
}
