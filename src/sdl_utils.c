#include "sdl_utils.h"

#include <stdlib.h>


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