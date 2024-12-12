#include "v810_mem.h"
#include "vb_set.h"
#include "patches.h"

typedef struct {
    const char gameid[6];
    uint8_t size;
    WORD address;
    const uint8_t *original;
    const uint8_t *patched;
} Patch;

static const Patch patches[] = {
    // Jack Bros. relies on draw timings to line up in a certain way
    // during level transitions. With the current implementation,
    // this lines up incorrectly, resulting in the next transition
    // being displayed for 1 frame at the end of a transition.
    // I believe this is caused by large VRAM copies occuring over one
    // allocated frame, which may take multiple actual frames.
    // To simulate this, we increase the delay by 2 frame.
    {
        .gameid = "EBVJBE",
        .address = 0x13714,
        .size = 2,
        .original = (const uint8_t[]){
            0x21, 0x40, // mov 1,r1
        },
        .patched = (const uint8_t[]){
            0x23, 0x40, // mov 3,r1
        },
    },
    {
        .gameid = "EBVJBJ",
        .address = 0x136e2,
        .size = 2,
        .original = (const uint8_t[]){
            0x21, 0x40, // mov 1,r1
        },
        .patched = (const uint8_t[]){
            0x23, 0x40, // mov 3,r1
        },
    },
    // Innsmouth no Yakata does a lot during its busywait that can be skipped.
    {
        .gameid = "8FVIMJ",
        .address = 0x19040,
        .size = 2,
        .original = (const uint8_t[]){
            0x44, 0x85, // be -0xbc
        },
        .patched = (const uint8_t[]){
            0xf0, 0x85, // be -0x10
        },
    },
    {
        .gameid = "8FVIME",
        .address = 0x19040,
        .size = 2,
        .original = (const uint8_t[]){
            0x44, 0x85, // be -0xbc
        },
        .patched = (const uint8_t[]){
            0xf0, 0x85, // be -0x10
        },
    },
    // Teleroboxer increments an otherwise unused memory value while busywaiting,
    // which messes with busywait detection.
    // Therefore, we patch out the increment.
    {
        .gameid = "01VTBJ",
        .address = 0x2e9dc,
        .size = 2,
        .original = (const uint8_t[]){
            0x61, 0x45, // add 1,r11
        },
        .patched = (const uint8_t[]){
            0x00, 0x00, // mov r0,r0
        },
    },
    // Virtual Boy Wario Land needs a similar patch.
    {
        .gameid = "01VWCJ",
        .address = 0x1c2cda,
        .size = 2,
        .original = (const uint8_t[]){
            0x61, 0x45, // add 1,r11
        },
        .patched = (const uint8_t[]){
            0x00, 0x00, // mov r0,r0
        }
    },
};

void apply_patches() {
    for (int i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
        char *gameid = (char*)(V810_ROM1.off + (V810_ROM1.highaddr & 0xFFFFFDF9));
        if (memcmp(tVBOpt.GAME_ID, patches[i].gameid, 6) == 0) {
            BYTE *patch_ptr = V810_ROM1.pmemory + patches[i].address;
            if (memcmp(patch_ptr, patches[i].original, patches[i].size) == 0) {
                memcpy(patch_ptr, patches[i].patched, patches[i].size);
            }
        }
    }
}