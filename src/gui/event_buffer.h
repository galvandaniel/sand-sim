/**
 * Module defining an interface for interacting with a ring buffer of SDL events
 * which supports synchronized access.
 */

#ifndef EVENT_BUFFER_H
#define EVENT_BUFFER_H

#include <SDL3/SDL.h>

/**
 * Max number of events which can be held by buffer. 
 * 
 * This value has been chosen empirically to accomodate as many events as SDL
 * normally processes from frame-to-frame while minimizing memory footprint.
 */
constexpr int MAX_BUFFERED_EVENTS = 16;

/**
 * Type defining 
 */
struct EventBuffer
{
    // Event storage.
    SDL_Event events[MAX_BUFFERED_EVENTS];

    // Number of currently stored events.
    int size;

    // Synchronization primitive for buffer access.
    SDL_Mutex *lock;
};


/**
 * Allocate an `EventBuffer` object whose element are all zeroed out.
 * 
 * The pointer returned by this function must be freed by `destroy_event_buffer()`
 * to avoid leaking memory.
 * 
 * @return Pointer to allocated EventBuffer object.
 */
struct EventBuffer *create_event_buffer();


/**
 * Free the memory held by the given `event_buffer` object.
 * 
 * Passing NULL to this function does nothing
 * 
 * @param event_buffer `EventBuffer` to deallocate.
 */
void destroy_event_buffer(struct EventBuffer *event_buffer);


/**
 * Atomically add an event to the end of `event_buffer`.
 * 
 * If several threads attempt to add to `event_buffer`, their access is
 * serialized.
 * 
 * @param event_buffer Buffer to mutate with new event appended to end.
 * @param event Event data to add to buffer.
 */
void event_buffer_append(struct EventBuffer *event_buffer, SDL_Event event);

/**
 * Atomically flush the buffer contents of the given `event_buffer` into `other`,
 * resetting `event_buffer` to empty.
 * 
 * Synchronization data is not copied over.
 * 
 * @param event_buffer Buffer containing event data to copy over and clear.
 * @param other Destination buffer to contain event data of `event_buffer`. 
 */
void event_buffer_flush(struct EventBuffer *event_buffer, struct EventBuffer *other);

#endif // EVENT_BUFFER_H