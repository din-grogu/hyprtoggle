# hyprtoggle

A native C++ plugin for **Hyprland** that introduces fully customizable window-snapping and trigger actions across **8 screen zones** when dragging windows, featuring a real-time **animated visual preview overlay**:

- **8 Customizable Screen Zones**: Define any behavior for `top`, `bottom`, `left`, `right`, `top_left`, `top_right`, `bottom_left`, and `bottom_right`.
- **Built-in Snap Geometries**: Snaps windows to fullscreen, halves (50%), quarter quadrants (25%), or centered floating.
- **Hyprland Dispatchers & Custom Commands**: Trigger any Hyprland dispatcher (e.g., `closewindow`, `killactive`, `movetoworkspace`) or arbitrary commands (`exec <cmd>`).
- **Live Visual Preview**: Displays a sleek translucent accent preview box with rounded borders indicating the target snap geometry or action zone.
- **Full Lua & Hyprlang Support**: Native configuration support in both `hyprland.lua` and traditional `hyprland.conf`.

---

## Installation via `hyprpm` (Recommended)

`hyprpm` (*Hyprland Plugin Manager*) handles fetching headers matching your active Hyprland compositor and recompiling plugins automatically.

### 1. Initialize `hyprpm`
```bash
hyprpm update
```

### 2. Add the repository
```bash
hyprpm add https://github.com/din-grogu/hyprtoggle
```

### 3. Enable the plugin
```bash
hyprpm enable hyprtoggle
```

### 4. Reload
```bash
hyprpm reload
```

---

## Configuration

### Hyprland Lua Configuration (`hyprland.lua`)

If you are using Hyprland's Lua configuration, customize `hyprtoggle` in your `hyprland.lua`:

```lua
-- Autostart hyprpm plugins on session startup
hl.exec_once("hyprpm reload -n")

-- Window dragging bind (Hyprland default)
local mainMod = "SUPER"
hl.bind(mainMod .. " + mouse:272", hl.dsp.window.drag(), { mouse = true })

-- Plugin Configuration
hl.config({
    plugin = {
        hyprtoggle = {
            enabled             = 1,
            threshold           = 20,    -- Distance in px from edge to trigger (default: 20)
            corner_threshold    = 60,    -- Corner zone size in px (default: 60)

            -- 8 Screen Zone Actions:
            action_top          = "fullscreen",       -- Options: "fullscreen", "maximize", "toggle_float", or any dispatcher
            action_bottom       = "toggle_float",     -- Options: "toggle_float", "closewindow", etc.
            action_left         = "snap_left",        -- Options: "snap_left", "closewindow", "killactive"
            action_right        = "snap_right",       -- Options: "snap_right"
            action_top_left     = "snap_top_left",    -- Options: "snap_top_left"
            action_top_right    = "snap_top_right",   -- Options: "snap_top_right"
            action_bottom_left  = "snap_bottom_left", -- Options: "snap_bottom_left"
            action_bottom_right = "snap_bottom_right",-- Options: "snap_bottom_right"

            -- Visual Preview Overlay
            preview             = 1,     -- 1 = enabled, 0 = disabled
            preview_rounding    = 10,    -- Corner rounding radius
            preview_border_size = 2,     -- Outline thickness
        },
    },
})
```

### Traditional `.conf` Configuration (`hyprland.conf`)

If you use classic `hyprland.conf` syntax:

```ini
exec-once = hyprpm reload -n

bindm = SUPER, mouse:272, movewindow

plugin {
    hyprtoggle {
        enabled = 1
        threshold = 20
        corner_threshold = 60

        # 8 Customizable Zone Actions:
        action_top          = fullscreen
        action_bottom       = toggle_float
        action_left         = snap_left
        action_right        = snap_right
        action_top_left     = snap_top_left
        action_top_right    = snap_top_right
        action_bottom_left  = snap_bottom_left
        action_bottom_right = snap_bottom_right

        # Visual Preview Overlay:
        preview = 1
        preview_rounding = 10
        preview_border_size = 2
    }
}
```

---

## Action Types Reference

Any of the 8 zone options (`action_top`, `action_bottom`, `action_left`, `action_right`, `action_top_left`, `action_top_right`, `action_bottom_left`, `action_bottom_right`) can be configured with:

### 1. Built-in Snap Geometries
| Action | Description |
| :--- | :--- |
| `"fullscreen"` / `"toggle_fullscreen"` | Toggles true fullscreen mode for the dragged window. |
| `"maximize"` | Maximizes the window within the monitor workarea (respects reserved gaps and bars). |
| `"toggle_float"` / `"togglefloating"` | Toggles floating mode on the window. |
| `"snap_left"` | Snaps the window to the left half (50% width) of the screen. |
| `"snap_right"` | Snaps the window to the right half (50% width) of the screen. |
| `"snap_top_left"` | Snaps the window to the top-left quarter (25% area). |
| `"snap_top_right"` | Snaps the window to the top-right quarter (25% area). |
| `"snap_bottom_left"` | Snaps the window to the bottom-left quarter (25% area). |
| `"snap_bottom_right"` | Snaps the window to the bottom-right quarter (25% area). |
| `"none"` | Disables action for that edge/corner. |

### 2. Arbitrary Hyprland Dispatchers & Custom Commands
You can specify any dispatcher name or command supported by Hyprland:
- `"closewindow"` or `"killactive"`: Closes the dragged window when dropped on that zone.
- `"dispatch movetoworkspace +1"`: Moves the window to the next workspace.
- `"exec notify-send 'Window Dropped'"`: Runs any shell command, desktop notification, or script.

---

## Configuration Options

| Option | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `enabled` | `int` | `1` | Enable or disable the plugin. |
| `threshold` | `int` | `20` | Distance in pixels from screen edges to trigger edge zones. |
| `corner_threshold` | `int` | `60` | Size in pixels of corner zones. |
| `action_top` | `string` | `"fullscreen"` | Action for the top edge. |
| `action_bottom` | `string` | `"toggle_float"` | Action for the bottom edge. |
| `action_left` | `string` | `"snap_left"` | Action for the left edge. |
| `action_right` | `string` | `"snap_right"` | Action for the right edge. |
| `action_top_left` | `string` | `"snap_top_left"` | Action for the top-left corner. |
| `action_top_right` | `string` | `"snap_top_right"` | Action for the top-right corner. |
| `action_bottom_left` | `string` | `"snap_bottom_left"` | Action for the bottom-left corner. |
| `action_bottom_right` | `string` | `"snap_bottom_right"` | Action for the bottom-right corner. |
| `preview` | `int` | `1` | Enable or disable the visual preview overlay. |
| `preview_rounding` | `int` | `10` | Corner radius for the preview rectangle. |
| `preview_border_size` | `int` | `2` | Border thickness for the preview rectangle. |

---

## Author

Created and maintained by **Ian Dieb** ([@din-grogu](https://github.com/din-grogu)).
Licensed under the MIT License.
