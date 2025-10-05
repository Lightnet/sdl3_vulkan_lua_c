

lua
```lua
local events = sdl.poll_events()
for i, event in ipairs(events) do
    if event.type == sdl.QUIT then
        print("Window closed.")
        return
    elseif event.type == sdl.EVENT_KEY_DOWN then

    end
end
```
lua
```
--SDL_EVENT_KEY_DOWN
sdl.EVENT_KEY_DOWN
```