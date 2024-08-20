# GUI sample
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/Core-2-Extreme/GUI_sample?color=darkgreen&style=flat-square)

## Index
* [What is this?????](https://github.com/Core-2-Extreme/GUI_sample#what-is-this)
* [What sample code does it have?????](https://github.com/Core-2-Extreme/GUI_sample#what-sample-code-does-it-have)
* [Build](https://github.com/Core-2-Extreme/GUI_sample#build)
* [License](https://github.com/Core-2-Extreme/GUI_sample#license)

## What is this?????
This is a sample application for 3ds homebrew (aka SDK). \
Homebrew application for 3ds such as [Video player for 3DS](https://github.com/Core-2-Extreme/Video_player_for_3DS) and [Battery mark for 3DS](https://github.com/Core-2-Extreme/Battery_mark_for_3DS) are based on this sample code.

## What sample code does it have?????
Currently this sample application contains following samples : 
* [Draw image sample](https://github.com/Core-2-Extreme/GUI_sample#draw-image-sample)
* [File explorer sample](https://github.com/Core-2-Extreme/GUI_sample#file-explorer-sample)
* [Hardware settings sample](https://github.com/Core-2-Extreme/GUI_sample#hardware-settings-sample)
* [Camera and mic sample](https://github.com/Core-2-Extreme/GUI_sample#camera-and-mic-sample)
* [Speaker sample](https://github.com/Core-2-Extreme/GUI_sample#speaker-sample)

### Draw image sample
This is a sample code to draw images such as .png and .jpg.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_0.png" width="400" height="480">

Application source code : [`source/sub_app0.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app0.cpp) \
Used APIs : 
* [`Draw_texture*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/draw/draw.h)
* [`Util_converter_convert_color()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.h)
* [`Util_curl_dl_data()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/curl.h)
* [`Util_decoder_image_decode*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/decoder.h)

### File explorer sample
This is a sample code to use file explorer.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_1.png" width="400" height="480">

Application source code : [`source/sub_app1.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app1.cpp) \
Used APIs : 
* [`Util_expl*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/expl.h)

### Hardware settings sample
This is a sample code to change hardware settings such as wifi state, screen brightness and sleep.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_2.png" width="400" height="480">

Application source code : [`source/sub_app2.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app2.cpp) \
Used APIs : 
* [`Util_hw_config*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/hw_config.h)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.h)

### Camera and mic sample
This is a sample code to take a picture by using camera and record sound by using mic.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_3.png" width="400" height="480">

Application source code : [`source/sub_app3.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app3.cpp) \
Used APIs : 
* [`Util_cam*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/cam.h)
* [`Util_converter_convert_color()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.h)
* [`Util_encoder*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/encoder.h)
* [`Util_mic*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/mic.h)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.h)

### Speaker sample
This is a sample code to playback audio.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_4.png" width="400" height="480">

Application source code : [`source/sub_app4.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app4.cpp) \
Used APIs : 
* [`Util_converter_convert_audio()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.h)
* [`Util_decoder*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/decoder.h)
* [`Util_speaker*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/speaker.h)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.h)

## Build
You need : 
* [devkitpro](https://devkitpro.org/wiki/Getting_Started)

If you want to build .cia, then you also need : 
* [bannertool](https://github.com/Steveice10/bannertool/releases) and [makerom](https://github.com/3DSGuy/Project_CTR/releases) (Copy them in your path e.g. in `{devkitPro_install_dir}\tools\bin`).

After installing devkitpro, type `{devkitPro_install_dir}\devkitARM\bin\arm-none-eabi-gcc -v`. \
You should see something like : 
```
.....
.....
.....
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 13.2.0 (devkitARM release 62)
```
Make sure you have release 62 or later. \
Note : Sometimes later versions have incompatibility, if you can't build with the newest tools, let me know. \
If you have older devkitpro, update it or compilation will fail.

* Clone this repository
  * On windows run `build.bat`
  * On other system, type `make` (`make -j` for faster build)

## License
This software is licensed as GNU General Public License v3.0.

Third party libraries are licensed as :

| Library | License |
| ------- | ------- |
| [Base64](https://github.com/ReneNyffenegger/cpp-base64/blob/master/LICENSE) | No specific license name               |
| [citro2d](https://github.com/devkitPro/citro2d/blob/master/LICENSE)         | zlib License                           |
| [citro3d](https://github.com/devkitPro/citro3d/blob/master/LICENSE)         | zlib License                           |
| [curl](https://github.com/curl/curl/blob/master/COPYING)                    | No specific license name               |
| [dav1d](https://github.com/videolan/dav1d/blob/master/COPYING)              | BSD 2-Clause                           |
| [ffmpeg](https://github.com/FFmpeg/FFmpeg/blob/master/COPYING.GPLv2)        | GNU General Public License v2.0        |
| [libctru](https://github.com/devkitPro/libctru#license)                     | zlib License                           |
| [mbedtls](https://github.com/Mbed-TLS/mbedtls/blob/development/LICENSE)     | Apache License 2.0                     |
| [mp3lame](https://github.com/gypified/libmp3lame/blob/master/COPYING)       | GNU Lesser General Public License v2.0 |
| [stb_image](https://github.com/nothings/stb/blob/master/LICENSE)            | Public Domain                          |
| [x264](https://github.com/mirror/x264/blob/master/COPYING)                  | GNU General Public License v2.0        |
| [zlib](https://github.com/madler/zlib/blob/master/LICENSE)                  | zlib License                           |
