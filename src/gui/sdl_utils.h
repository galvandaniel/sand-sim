#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <SDL3/SDL.h>

/**
 * Wrapper around all SDL API calls to check for and report on failure.
 */
#define SDL_CHECK(sdl_result) _Generic((sdl_result), \
                                        bool: sdl_check_bool, \
                                        const SDL_PixelFormatDetails *: sdl_check_ptr_to_const, \
                                        default: sdl_check_ptr \
                                      )(sdl_result, __FILE__, __LINE__)

/**
 * Various color constants.
 */
constexpr SDL_Color NO_COLOR = {.r = 0, .g = 0, .b = 0, .a = 0};
constexpr SDL_Color RED = {.r = 255, .g = 0, .b = 0, .a = 255};
constexpr SDL_Color WHITE = {.r = 255, .g = 255, .b = 255, .a = 255};
constexpr SDL_Color BLACK = {.r = 0, .g = 0, .b = 0, .a = 255};
constexpr SDL_PixelFormat ALPHA_PIXEL_FORMAT = SDL_PIXELFORMAT_RGBA32;


/**
 * Get the RGB value of the surface pixel located at the given coordinates.
 * 
 * @param surface Surface to read pixel data from.
 * @param surface_coords (x,y) coordinates within surface to fetch pixel from.
 * @return Pixel RGB data packed into an SDL_Color.
 */
SDL_Color get_pixel(SDL_Surface *surface, SDL_Point surface_coords);


/**
 * Get whether VSync of any kind (standard, interval, adaptive) is enabled
 * on the given renderer.
 * 
 * @param renderer Renderer to query for VSync enabled.
 * @return True if VSync is enabled on the given renderer, false otherwise.
 */
bool is_vsync_enabled(SDL_Renderer *renderer);


/**
 * Given the filename for a location to a JPG or PNG, load the image as an
 * SDL_Texture on the given renderer.
 *
 * @param app Renderer to load image on.
 * @param filename Filepath of image to load from, relative to program location.
 * @return Image loaded as SDL_Texture
 */
SDL_Texture *load_texture(SDL_Renderer *renderer, const char *filename);



/**
 * Given the filename of a JPG or PNG, load the image as an SDL_Texture on the
 * given renderer with the given alpha value set.
 * 
 * The loaded texture will have alpha blending enabled for drawing with opacity.
 * This will force the resulting texture to have an SDL pixel format which
 * supports an alpha channel.
 * 
 * @param app Renderer to load image on.
 * @param filename Filepath of image to load from, relative to program location.
 * @param alpha Alpha value of returned SDL_Texture for displaying with opacity.
 * @return Transparent image loaded as SDL_Texture.
 */
SDL_Texture *load_texture_alpha(SDL_Renderer *renderer, const char *filename, unsigned char alpha);


/**
 * Wrapper for all SDL calls which can fail on returning false.
 * 
 * Not intended to be called on anything other than SDL API results.
 * 
 * @param result Bool result from an SDL API call.
 * @param file The expanded value of __FILE__ when API call is made.
 * @param line The expanded value of __LINE__ when API call is made.
 * @return True on successful SDL API call, False on any SDL error.
 */
bool sdl_check_bool(bool result, const char *file, int line);

/**
 * Wrapper for all SDL calls which can fail on returning a NULL pointer.
 * 
 * Not intended to be called on anything other than SDL API results.
 * 
 * @param sdl_ptr A pointer to an SDL type as returned from an SDL API call.
 * @param file The expanded value of __FILE__ when API call is made.
 * @param line The expanded value of __LINE__ when API call is made.
 * @return sdl_ptr as it is passed to this function.
 */
void *sdl_check_ptr(void *sdl_ptr, const char *file, int line);


/**
 * Wrapper for all SDL calls which can fail on returning a NULL pointer.
 * 
 * Not intended to be called on anything other than SDL API results.
 * 
 * @param sdl_ptr A pointer to an SDL type as returned from an SDL API call.
 * @param file The expanded value of __FILE__ when API call is made.
 * @param line The expanded value of __LINE__ when API call is made.
 * @return sdl_ptr as it is passed to this function.
 */
const void *sdl_check_ptr_to_const(const void *sdl_ptr, const char *file, int line);

#endif // SDL_UTILS_H