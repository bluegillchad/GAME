#ifndef textureE_H
#define textureE_H

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

    void loadTexture(std::string path, SDL_Renderer* &renderer);

    void loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* &textFont, SDL_Renderer* &renderer);

    void free();

    // render at position with rotation and flipping
    void render (SDL_Renderer* &renderer, SDL_Rect* clip = NULL,
                 double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );
};

#endif // textureE_H

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

void textureE::loadTexture(std::string path, SDL_Renderer* &renderer)
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

textureE::void loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* &textFont, SDL_Renderer* &renderer)
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
    }*/

textureE::void free()
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
textureE::void render (SDL_Renderer* &renderer, SDL_Rect* clip = NULL,
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
