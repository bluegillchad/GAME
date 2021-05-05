#ifndef SDL_stuff_H
#define SDL_stuff_H

#include "game.h"
enum errorType
{
    IMG_Err,
    SDL_Err,
    MIX_Err,
    TTF_Err,
    none
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

    textureE();

    textureE(int posX_, int posY_);

    void loadTexture(std::string path, SDL_Renderer* &renderer, const bool &isColorKey);

    void loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* &textFont, SDL_Renderer* &renderer);

    void free();

    // render at position with rotation and flipping
    void render (SDL_Renderer* &renderer, SDL_Rect* clip = NULL,
                 double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );
};

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer);

void quitSDL(SDL_Window* &window, SDL_Renderer* &renderer);

void renderText(const std::string &text, const SDL_Color &textColor, TTF_Font* &textFont, SDL_Renderer* &renderer,
                textureE &texture, const int &posX, const int &posY);

void changeFontSize(TTF_Font* &textFont, const int &newSize);

textureE::textureE()
{
    texture = NULL;
    width = 0;
    height = 0;
    posX = 0;
    posY = 0;
}

textureE::textureE(int posX_, int posY_)
{
    texture = NULL;
    width = 0;
    height = 0;
    posX = posX_;
    posY = posY_;
}

void textureE::loadTexture(std::string path, SDL_Renderer* &renderer, const bool &isColorKey)
{
    free();
    SDL_Texture* newTexture;
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL ) logSDLError(std::cout, "Unable to load image", true, IMG_Err);
    else
        {
            if (isColorKey) SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface -> format, 0xFF, 0xFF, 0xFF));
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

void textureE::loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* &textFont, SDL_Renderer* &renderer)
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

void textureE::free()
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
            os << msg << std::endl; break;
    }
    if (fatal) {
        SDL_Quit();
        IMG_Quit();
        exit(1);
    }
}

void textureE::render (SDL_Renderer* &renderer, SDL_Rect* clip /*= NULL*/,
                 double angle /*= 0.0*/, SDL_Point* center /*= NULL*/, SDL_RendererFlip flip /*= SDL_FLIP_NONE*/ )
{
    SDL_Rect renderPos = {posX, posY, width, height};
    if (clip != NULL)
    {
        renderPos.w = clip->w;
        renderPos.h = clip->h;
    }
    SDL_RenderCopyEx(renderer, texture, clip, &renderPos, angle, center, flip);
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

void renderText(const std::string &text, const SDL_Color &textColor, TTF_Font* &textFont, SDL_Renderer* &renderer,
                textureE &texture, const int &posX, const int &posY)
{
    texture.loadFromRenderedText(text, textColor, textFont, renderer);
    texture.posX = posX;
    texture.posY = posY;
    texture.render(renderer);
}

void changeFontSize(TTF_Font* &textFont, const int &newSize, const char* fontPath)
{
    if (textFont != NULL) TTF_CloseFont(textFont);
    textFont = TTF_OpenFont(fontPath, newSize);
}
#endif // SDL_stuff_H
