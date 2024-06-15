# Mixpult
## [deej](https://github.com/omriharel/deej) app for Windows rewritten in C++ with some extra stuff
**VERY** lightweight. Uses around 5MB of RAM and 0.1% CPU (Ryzen 5 5500)

Added features:
1. Buttons
   - Each slider has a button that can toggle mute by sending `m` instead of *`value`*
   - Buttons have some LEDs with preset colors that change to red when muted
2. Ping
   - Device can ping the host PC to tell if the app is still running (if PC is on)
   - The button LEDs were still on even if i turned my PC off. So now when the device doesn't recieve a response to a ping it turns the lights off

## ADD:
- [ ] Better smoothing of slider values
- [ ] New icon
- [ ] Send session events to device (muting)
- [ ] Suppont for buttons with more functions (spotify track skipping)
- [ ] Ability to change button color over serial
- [ ] Code cleanup
- [ ] Maybe support for motorized sliders

## FIX:
- [x] Session discovery
- [ ] Session releasing

---
Don't look at the code too much 😅. I've never written a Windows app and it shows.
