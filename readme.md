# capturer

there are several projects in the repository:
- capturer - main project. it allows you to capture video stream with the ability to simultaneously view the input signal. it has low latency, which is quite useful when recording a gameplay videos
- android_ctrl - project, used to remotely control the program using Android Phone
- remote_audio_output - project, used to transmit sound over the LAN

### important note
this project requires a capture card that supports the Decklink interface. though capture cards with video4linux/dshow interfaces are supported, project may not work well depending on specific card

### building the projects
all programs are written using Qt5 libraries. therefore they require the Qt Creator IDE. it is also recommended that you disable the shadow build option after opening the project. build this project like any typical qt-project, however there are some issues that you have to keep in mind while building the «capturer» project:

- it is strongly recommended to build the 64-bit version of the program. 32-bit version will almost certainly crash with std::bad_alloc exception when trying to start recording - there is some witchcraft involved, which depends on the specific combinations of resolution, pixel format and selected encoder

- if you want to control the program from your Android Phone through capturer_ctrl, then run ./externals/3rdparty/http_server_update. script (using sh/cmd) to download the repositories with the http-server implementation

##### building projects for Linux
- download "Desktop Video SDK" from https://www.blackmagicdesign.com/support
- extract sdk to the ./externals/3rdparty/blackmagic_decklink_sdk folder
- build ffmpeg - run the script build.sh from the directory ./externals/3rdparty/ffmpeg 
<br>take note, that this script requires following packages to work: <b>autoconf automake libtool build-essential cmake git subversion mercurial</b>
<br>this script was tested on Ubuntu 16.04, 17.10 and Fedora 25-26, but there are times, when combination of geomagnetic fluctuations, unusual temperature on the Mars' Poles and unknown bloody bullshit prevent script from executing. well, I wish thee good luck then

##### building projects for Windows
- download ffmpeg's shared/dev package from https://github.com/BtbN/FFmpeg-Builds/releases (ends with "-win64-(l)gpl-shared.zip")
- extract "include" and "lib" folders to ./externals/3rdparty/ffmpeg/win
- extract bin/*.dll's to ./bin
- pray to your deity
- build it!
