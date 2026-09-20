#if !defined(__NC_AUDIO_H__)
#define __NC_AUDIO_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_sync.h"
#include "nc_time.h"

// @defines____________________________________________________________________
#define AUDIO_CHANNEL_COUNT 2                                                   // TODO: maybe consider 5.1, 7.1, and Atmos

typedef u32 AudioInfoFlag;
enum : u32 {
    AUDIO_INFO_FLAG_NONE   = 0,
    AUDIO_INFO_FLAG_LOOP   = (1 << 0),
    AUDIO_INFO_FLAG_STREAM = (1 << 1),
    AUDIO_INFO_FLAG_COUNT
};

typedef u32 AudioFormatKind;
enum : u32 {
    AUDIO_FORMAT_PCM_INVALID = 0,
    AUDIO_FORMAT_PCM_INT16   = (1 << 0),
    AUDIO_FORMAT_PCM_INT24   = (1 << 1),
    AUDIO_FORMAT_PCM_INT32   = (1 << 2),
    AUDIO_FORMAT_IEEE_F32    = (1 << 3),
    AUDIO_FORMAT_NON_PCM     = (1 << 4),
    AUDIO_FORMAT_COUNT
};

// @types______________________________________________________________________
struct AudioHandle {
    u64 V[2];
};

struct AudioInfo {
    f32             Volume[AUDIO_CHANNEL_COUNT];
    f32             Pitch;
    AudioInfoFlag   Flags;
    AudioFormatKind Format;
    b32             IsPaused;
    u64             FrameCurrent;
    u64             FrameCount;
    u32             SampleRate;
    u32             Channels;
};

struct AudioStream {
    Arena* MemPool;
};

struct AudioStreamNode {
    AudioStreamNode* Next;
    AudioStreamNode* Prev;
    AudioStream      V;
};

struct AudioStreamSlot {
    AudioStreamNode* Head;
    AudioStreamNode* Tail;
};

struct AudioState {
    Arena*           MemPool;
    StripeArray      StreamStripes;
    AudioStreamSlot* StreamSlots;
    u64              StreamSlotsCount;
    AudioStream*     FreeStream;
    f32              MasterVolume;
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________

#endif // __NC_AUDIO_H__
