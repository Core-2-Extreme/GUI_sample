# GUI sample
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/Core-2-Extreme/GUI_sample?color=darkgreen&style=flat-square)

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
* [`Draw_texture*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/draw/draw.hpp)
* [`Util_converter_convert_color()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.hpp)
* [`Util_curl_dl_data()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/curl.hpp)
* [`Util_image_decoder_decode()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/decoder.hpp)

### File explorer sample
This is a sample code to use file explorer.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_1.png" width="400" height="480">

Application source code : [`source/sub_app1.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app1.cpp) \
Used APIs : 
* [`Util_expl*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/explorer.hpp)

### Hardware settings sample
This is a sample code to change hardware settings such as wifi state, screen brightness and sleep.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_2.png" width="400" height="480">

Application source code : [`source/sub_app2.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app2.cpp) \
Used APIs : 
* [`Util_cset*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/change_setting.hpp)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.hpp)

### Camera and mic sample
This is a sample code to take a picture by using camera and record sound by using mic.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_3.png" width="400" height="480">

Application source code : [`source/sub_app3.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app3.cpp) \
Used APIs : 
* [`Util_cam*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/camera.hpp)
* [`Util_converter_convert_color()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.hpp)
* [`Util_encoder*(), Util_audio_encoder*(), Util_image_encoder_encode()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/encoder.hpp)
* [`Util_mic*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/mic.hpp)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.hpp)

### Speaker sample
This is a sample code to playback audio.

<img src="https://raw.githubusercontent.com/Core-2-Extreme/GUI_sample/main/screenshots/sample_app_4.png" width="400" height="480">

Application source code : [`source/sub_app4.cpp`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/source/sub_app4.cpp) \
Used APIs : 
* [`Util_converter_convert_audio()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/converter.hpp)
* [`Util_audio_decoder*(), Util_decoder*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/decoder.hpp)
* [`Util_speaker*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/speaker.hpp)
* [`Util_queue*()`](https://github.com/Core-2-Extreme/GUI_sample/blob/main/include/system/util/queue.hpp)

## Build
You need : 
* [devkitpro](https://devkitpro.org/wiki/Getting_Started)

If you want to build .cia, then you also need : 
* [bannertool](https://github.com/Steveice10/bannertool/releases) and [makerom](https://github.com/3DSGuy/Project_CTR/releases) (Copy them in your path e.g. in `{devkitPro_install_dir}\tools\bin`).

If you already have devkitpro, type `{devkitPro_install_dir}\devkitARM\bin\arm-none-eabi-gcc -v`. \
You should see something like : 
```
.....
.....
.....
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 12.2.0 (devkitARM release 60)
```
Make sure you have release 60 or later. \
If you have older devkitpro, update it or compilation will fail.

* Clone this repository
  * On windows run `build.bat`
  * On other system, type `make` (`make -j` for faster build)
