Running Conquer Space on WSL2
---

Getting Conquer Space to work on WSL is very possible but you need to install the right version of stuff

Run `glxinfo -B` to check the opengl version. Make sure it's 4.3 or above.

Run `export LIBGL_ALWAYS_INDIRECT=0` and `export DISPLAY=:0`

https://stackoverflow.com/questions/66497147/cant-run-opengl-on-wsl2?rq=3 
