# hyprtoggle

A native C++ plugin for **Hyprland** that introduces edge-snapping actions when dragging windows:
- **Drag to top edge**: Automatically makes the dragged window **fullscreen** (or maximized).
- **Drag down from fullscreen**: Hyprland natively un-fullscreens and restores the original floating dimensions under the cursor.
- **Modular edge architecture**: Designed cleanly to support additional edge triggers (such as dragging to the bottom edge to toggle floating window mode).

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
            enabled   = true,  -- Enable or disable the plugin (default: true)
            mode      = 2,     -- 2 = Real Fullscreen (default), 1 = Maximized (keeps bars/gaps)
            threshold = 25,    -- Distance in pixels from screen edge to trigger (default: 25)
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
        threshold = 25
    }
}
```

---

## Manual Build (Without `hyprpm`)

To build the plugin shared object manually:

```bash
make host
```
The compiled plugin will be available at `./hyprtoggle.so`.

You can test-load it dynamically using `hyprctl`:
```bash
hyprctl plugin load $(pwd)/hyprtoggle.so
hyprctl plugin unload $(pwd)/hyprtoggle.so
```

---

## Configuration Reference

| Option | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `plugin:hyprtoggle:enabled` | `int` / `bool` | `1` (`true`) | Enable or disable plugin functionality. |
| `plugin:hyprtoggle:mode` | `int` | `2` | `1`: Maximized (respects bars/gaps), `2`: Real fullscreen. |
| `plugin:hyprtoggle:threshold` | `int` | `25` | Activation distance in pixels from the monitor's top boundary. |

---

## Author

Created and maintained by **Ian Dieb** ([@din-grogu](https://github.com/din-grogu)).
Licensed under the MIT License.
