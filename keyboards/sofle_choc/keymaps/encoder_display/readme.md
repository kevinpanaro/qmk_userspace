# Sofle Choc - Encoder Display Keymap

This keymap extends the base Sofle Choc functionality with **encoder action display** on the OLED screens, with each encoder controlling its own display independently.

## Features

### Enhanced OLED Display
- **Layer information** - Current active layer on both displays
- **Independent encoder tracking** - Each encoder only updates its own OLED
- **Left encoder → Left OLED** - Master side encoder actions shown on left display
- **Right encoder → Right OLED** - Slave side encoder actions shown on right display
- **Temporary brightness display** - Shows brightness level when changed (synced to both displays)

### Display Layout

#### Normal State:
```
macos
     
     
```

#### After Volume Change (Left Encoder → Left OLED only):
```
Left OLED:        Right OLED:
macos             macos
VOL+
                       
```

#### After Debug Action (Right Encoder → Right OLED only):
```
Left OLED:        Right OLED:
macos             macos
                  STEP
                       
```

#### After OLED Brightness Change (Synced to both displays):
```
Left OLED:        Right OLED:
macos             macos
OLD+              OLD+
128               128
```

## Encoder Functions

### Encoder 0 (Left)
- **Default**: Volume control (VOL+/VOL-)
- **With Cmd**: System brightness (BRT+/BRT-)
- **LOWER layer**: OLED brightness (OLD+/OLD-)

### Encoder 1 (Right)
- **Default**: Debug step controls (STEP/OVER)
- **With Cmd**: Debug continue/step out (CONT/STEP)
- **With Alt**: Word navigation (WRD>/\<WRD)
- **UPPER layer**: OLED brightness (OLD+/OLD-)

## Display Codes

| Code  | Meaning |
|-------|---------|
| VOL+  | Volume Up |
| VOL-  | Volume Down |
| BRT+  | System Brightness Up |
| BRT-  | System Brightness Down |
| OLD+  | OLED Brightness Up |
| OLD-  | OLED Brightness Down |
| CONT  | Debug Continue |
| STEP  | Debug Step |
| OVER  | Debug Step Over |
| WRD>  | Next Word |
| \<WRD | Previous Word |
| SYS   | System Brightness (level unknown) |
| 128   | OLED Brightness Level (0-255) |

## Technical Details

- **Display timeout**: 3 seconds
- **OLED brightness tracking**: Shows exact level (0-255)
- **System brightness tracking**: Shows action only (level unknown)
- **Real-time updates**: Actions appear immediately on OLED
- **Split sync enabled**: Activity timestamps synchronized between halves
- **OLED rotation**: Both displays rotated 270° for consistent orientation

## Split Keyboard Configuration

This keymap includes the following split sync options for optimal functionality:

```c
#define SPLIT_TRANSPORT_MIRROR    // RGB matrix effects sync
#define SPLIT_LAYER_STATE_ENABLE  // Layer state sync for OLED
#define SPLIT_LED_STATE_ENABLE    // LED status sync (caps lock, etc.)
#define SPLIT_OLED_ENABLE         // OLED on/off state sync
#define SPLIT_ACTIVITY_ENABLE     // Activity timestamp sync for encoder timeouts
#define SPLIT_TRANSACTION_IDS_USER ENCODER_SYNC  // Custom encoder data sync
```

### Custom Data Sync

The keymap implements custom data synchronization between master and slave halves:

**Why sync is needed:**
- In QMK split keyboards, ALL encoder processing happens on the master side (USB-connected half)
- Both encoder 0 (left/master) and encoder 1 (right/slave) are processed by the master controller
- Without sync, the slave OLED cannot display encoder 1 actions because the data only exists on master

**What's synced:**
- Encoder actions for both encoders (display text, timestamps, keycode)
- Brightness level, timestamps, and brightness type (OLED vs system)

**Sync strategy:**
- **Immediate sync** when any encoder is turned (responsive visual feedback)
- **Dirty flag checking** prevents redundant transmissions
- **Fallback sync** every 1 second ensures reliability
- **No sync when idle** (efficient bandwidth usage)

**Technical details:**
- **Sync method**: One-way transaction (master → slave) using `transaction_rpc_send()`
- **Data size**: 45 bytes total (38 bytes encoder data + 7 bytes brightness)
- **Buffer size**: 48 bytes each direction
- **Implementation**: QMK's split transaction system with change detection

## Build Instructions

```bash
qmk compile -kb sofle_choc -km encoder_display
```

## Based On

This keymap is based on the `dev` keymap with added encoder display functionality.
