#pragma once

// Layer enum
enum layer_names {
    MACOS,
    PC,
    GAME,
    VLRNT,
#if defined(TRI_LAYER_ENABLE)
    LOWER = 29,
    UPPER = 30,
    ADJUST = 31,
#endif
};
