#include "gui/event_buffer.h"
#include "gui/sdl_utils.h"
#include "utils.h"

#include <SDL3/SDL.h>

#include <stdlib.h>

// Maximum size of a string which is to describe an `SDL_Event`.
constexpr int MAX_EVENT_DESCRIPTION_LENGTH = 256;


struct EventBuffer *create_event_buffer()
{
    // Cannot continue if SDL was not initialized first.
    if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_Log("\nWARNING: %s:%d\nReason: Attempted to create_event_buffer() without SDL_Init()!\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    struct EventBuffer *event_buffer = SAFE_CALLOC(1, sizeof(*event_buffer));
    event_buffer->lock = SDL_CHECK(SDL_CreateMutex());
    return event_buffer;
}


void destroy_event_buffer(struct EventBuffer *event_buffer)
{
    if (event_buffer == nullptr)
    {
        return;
    }

    SDL_DestroyMutex(event_buffer->lock);
    free(event_buffer);
}


void event_buffer_append(struct EventBuffer *event_buffer, SDL_Event event)
{
    if (event_buffer == nullptr)
    {
        SDL_Log("\nWARNING: %s:%d\nReason: Attempted to append to NULL EventBuffer!\n", __FILE__, __LINE__);
        return;
    }

    // Atomically modify buffer.
    SDL_LockMutex(event_buffer->lock);
    int index_of_next_item = event_buffer->size;
    // If we cannot append any more events, log the event which has been lost.
    if (index_of_next_item >= MAX_BUFFERED_EVENTS)
    {
        char event_desc[MAX_EVENT_DESCRIPTION_LENGTH];
        SDL_GetEventDescription(&event, event_desc, MAX_EVENT_DESCRIPTION_LENGTH);
        SDL_Log("\nWARNING: %s:%d\nReason: EventBuffer at max capacity (%d), cannot add event: %s\n", __FILE__, __LINE__, MAX_BUFFERED_EVENTS, event_desc);
        return;
    }

    event_buffer->events[index_of_next_item] = event;
    ++event_buffer->size;
    SDL_UnlockMutex(event_buffer->lock);
}


/**
 * Atomically copy the buffer contents of the given `event_buffer` into `other`.
 * 
 * Synchronization data is not copied over.
 */
void event_buffer_flush(struct EventBuffer *event_buffer, struct EventBuffer *other)
{
    // Atomically access buffer contents.
    SDL_LockMutex(event_buffer->lock);
    memcpy(other->events, event_buffer->events, sizeof(event_buffer->events));
    other->size = event_buffer->size;
    event_buffer->size = 0;
    SDL_UnlockMutex(event_buffer->lock);
}