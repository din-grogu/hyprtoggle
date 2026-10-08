# hyprtoggle

A native C++ plugin for **Hyprland** that introduces edge-snapping and corner-snapping actions when dragging windows, complete with an **animated visual snap preview overlay**:

- **Top Edge**: Snaps the window to **fullscreen** (or maximized).
- **Bottom Edge**: Toggles the window between **tiled** and **floating**.
- **Left & Right Edges**: Snaps the window to the **left or right half (50%)** of the screen.
- **Four Corners (`Top-Left`, `Top-Right`, `Bottom-Left`, `Bottom-Right`)**: Snaps the window into **quarter quadrants (25%)**.
- **Live Visual Preview**: Shows a sleek, translucent accent preview box with rounded corners and outline while hovering over any snap zone.

---

## Installation via `hyprpm` (Recommended)

`hyprpm` (*Hyprland Plugin Manager*) handles fetching the exact headers matching your active Hyprland compositor commit and rebuilding plugins automatically during system updates.

### 1. Initialize `hyprpm` (if not done previously)
```bash
hyprpm update
```
*(On first execution, this may ask for `sudo` to initialize `/var/cache/hyprpm`)*.

### 2. Add the repository
```bash
hyprpm add https://github.com/din-grogu/hyprtoggle
```
*Or from a local clone:*
```bash
hyprpm add /path/to/hyprtoggle
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

## Autostart & Configuration

### Hyprland Lua Configuration (`hyprland.lua`)

Add the following to your `hyprland.lua`:

```lua
-- Autostart hyprpm plugins on session startup
hl.exec_once("hyprpm reload -n")

-- Window dragging bind (Hyprland default)
local mainMod = "SUPER"
hl.bind(mainMod .. " + mouse:272", hl.dsp.window.drag(), { mouse = true })

-- Optional: Plugin customization
hl.config({
    plugin = {
        hyprtoggle = {
            enabled             = true,  -- Enable or disable the plugin (default: true)
            mode                = 2,     -- 2 = Real Fullscreen (default), 1 = Maximized (keeps bars/gaps)
            threshold           = 20,    -- Edge activation distance in pixels (default: 20)
            corner_threshold    = 60,    -- Corner activation distance in pixels (default: 60)
            preview             = true,  -- Enable visual snap preview overlay (default: true)
            preview_rounding    = 10,    -- Corner radius of preview overlay (default: 10)
            preview_border_size = 2,     -- Outline thickness of preview overlay (default: 2)
        },
    },
})
```

### Traditional `.conf` Configuration (`hyprland.conf`)

If you are using classic `.conf` syntax:

```ini
exec-once = hyprpm reload -n

bindm = SUPER, mouse:272, movewindow

plugin {
    hyprtoggle {
        enabled = 1
        mode = 2
        threshold = 20
        corner_threshold = 60
        preview = 1
        preview_rounding = 10
        preview_border_size = 2
    }
}
```

---

## Configuration Reference

| Option | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `plugin:hyprtoggle:enabled` | `int` / `bool` | `1` (`true`) | Enable or disable plugin functionality. |
| `plugin:hyprtoggle:mode` | `int` | `2` | Top edge mode: `1`: Maximized (respects bars/gaps), `2`: Real fullscreen. |
| `plugin:hyprtoggle:threshold` | `int` | `20` | Distance in pixels from screen borders to trigger edge snapping. |
| `plugin:hyprtoggle:corner_threshold` | `int` | `60` | Distance in pixels from screen corners to trigger corner quadrant snapping. |
| `plugin:hyprtoggle:preview` | `int` / `bool` | `1` (`true`) | Enable or disable the translucent visual preview overlay. |
| `plugin:hyprtoggle:preview_rounding` | `int` | `10` | Corner radius for the preview rectangle. |
| `plugin:hyprtoggle:preview_border_size` | `int` | `2` | Border stroke width for the preview rectangle. |

---

## Snap Zones Reference

| Zone | Trigger Region | Action Result |
| :--- | :--- | :--- |
| **`TOP`** | Top border (center) | Toggles Fullscreen / Maximized |
| **`BOTTOM`** | Bottom border (center) | Toggles Floating / Tiled |
| **`LEFT`** | Left border (center) | Snaps to Left Half (50% width) |
| **`RIGHT`** | Right border (center) | Snaps to Right Half (50% width) |
| **`TOP_LEFT`** | Top-left corner | Snaps to Top-Left Quadrant (25% area) |
| **`TOP_RIGHT`** | Top-right corner | Snaps to Top-Right Quadrant (25% area) |
| **`BOTTOM_LEFT`** | Bottom-left corner | Snaps to Bottom-Left Quadrant (25% area) |
| **`BOTTOM_RIGHT`**| Bottom-right corner | Snaps to Bottom-Right Quadrant (25% area) |

---

## Author

Created and maintained by **Ian Dieb** ([@din-grogu](https://github.com/din-grogu)).
Licensed under the MIT License.
