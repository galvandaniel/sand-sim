#include "gui/sdl_utils.h"

#include <SDL3/SDL_render.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>


SDL_Color get_pixel(SDL_Surface *surface, SDL_Point surface_coords)
{
    SDL_Color result;
    SDL_CHECK(SDL_ReadSurfacePixel(surface, 
                                   surface_coords.x, 
                                   surface_coords.y, 
                                   &result.r, 
                                   &result.g, 
                                   &result.b, 
                                   &result.a)
                                  );
    return result;
}


bool is_vsync_enabled(SDL_Renderer *renderer)
{
    int vsync_flags;
    SDL_CHECK(SDL_GetRenderVSync(renderer, &vsync_flags));
    return vsync_flags != SDL_RENDERER_VSYNC_DISABLED;
}


SDL_Texture *load_texture(SDL_Renderer *renderer, const char *filename)
{
    // Call to SDL_image to load image.
    SDL_Texture *texture = SDL_CHECK(IMG_LoadTexture(renderer, filename));
    return texture;
}


// Currently unused, but maybe useful in future?
SDL_Texture *load_texture_alpha(SDL_Renderer *renderer, const char *filename, unsigned char alpha)
{
    const SDL_PixelFormatDetails *alpha_format_details = SDL_CHECK(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32));

    // SDL_Image makes no guarantee on the image format of loaded textures.
    //
    // To guarantee alpha channel presence, temporarily load image as surface,
    // then convert surface to a portable alpha channel format. Finally, convert
    // to texture, enable + set alpha blending, and free the surfaces.
    SDL_Surface *raw_surface = SDL_CHECK(IMG_Load(filename));
    SDL_Surface *alpha_surface = SDL_CHECK(SDL_ConvertSurface(raw_surface, alpha_format_details->format));
    SDL_Texture *texture = SDL_CHECK(SDL_CreateTextureFromSurface(renderer, alpha_surface));
    SDL_CHECK(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND));
    SDL_CHECK(SDL_SetTextureAlphaMod(texture, alpha));

    SDL_DestroySurface(raw_surface);
    SDL_DestroySurface(alpha_surface);
    return texture;   
}


bool sdl_check_bool(bool result, const char *file, int line)
{
    if (!result)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "\nSDL FAILURE: %s:%d\nReason: %s\n",
                     file,
                     line,
                     SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return result;
}


void *sdl_check_ptr(void *sdl_ptr, const char *file, int line)
{
   if (sdl_ptr == nullptr)
   {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "\nSDL FAILURE: %s:%d\nReason: %s\n",
                    file,
                    line,
                    SDL_GetError());
       exit(EXIT_FAILURE);
   }
   return sdl_ptr;
}


const void *sdl_check_ptr_to_const(const void *sdl_ptr, const char *file, int line)
{
   if (sdl_ptr == nullptr)
   {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "\nSDL FAILURE: %s:%d\nReason: %s\n",
                    file,
                    line,
                    SDL_GetError());
       exit(EXIT_FAILURE);
   }
   return sdl_ptr;
}