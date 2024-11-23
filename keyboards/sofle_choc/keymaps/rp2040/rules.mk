# RP2040 microcontroller
CONVERT_TO = rp2040_ce

ENCODER_MAP_ENABLE = yes

# Tri Layers
TRI_LAYER_ENABLE = yes

# Key Overrides : https://docs.qmk.fm/features/key_overrides
# layer switching when command + fn key is pressed
KEY_OVERRIDE_ENABLE = no

# Combos : https://docs.qmk.fm/features/combo
# monentary layer switch when held
COMBO_ENABLE = no

# OS Detection : https://docs.qmk.fm/features/os_detection
OS_DETECTION_ENABLE = yes

# OLED Driver : https://docs.qmk.fm/features/oled_driver
OLED_ENABLE = yes

# WPM Calculation : https://docs.qmk.fm/features/wpm
WPM_ENABLE = no

# RGB Matrix : https://docs.qmk.fm/features/rgb_matrix
# Custom : https://docs.qmk.fm/features/rgb_matrix#custom-rgb-matrix-effects
RGB_MATRIX_CUSTOM_USER = yes

# Encoders : https://docs.qmk.fm/features/encoders
# Better encoders to support modifiers
ENCODER_MAP_ENABLE = no
ENCODER_ENABLE = yes

# Globe Key : https://skip.house/blog/qmk-globe-key
# https://gist.github.com/lordpixel23/87498dc42e328eabdff6dd258a667efd
KEYBOARD_SHARED_EP = no
