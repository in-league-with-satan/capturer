FROM ubuntu:20.04

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update && apt install -y gcc g++ make cmake autoconf automake libtool git pkg-config wget xz-utils libfreetype6-dev libfontconfig1-dev libpulse-dev libva-dev p7zip-full python \
        mesa-common-dev libgl1-mesa-dev libxi-dev libxcomposite-dev libxcursor-dev libxtst-dev libxkbcommon-dev libxkbcommon-x11-dev \
        libgles2-mesa-dev libwayland-dev libwayland-egl-backend-dev libncurses5-dev libncursesw5-dev libx11-dev libxcb1-dev \
        libxext-dev libxfixes-dev '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev \
    && mkdir /qt && cd /qt \
    && export qt_ver_f=`git ls-remote --tags https://github.com/qt/qt5 | awk '{print $2}' | grep -v '\^{}$' | grep -v '-' | sort -V | tail -1 | awk '{split($0, a, "/"); print a[3]}' | awk '{split($0, a, "v"); print a[2]}'` \
    ; export qt_ver_s=`echo $qt_ver_f | awk '{split($0, a, "."); printf("%s.%s", a[1], a[2])}'` \
    ; wget http://download.qt.io/archive/qt/$qt_ver_s/$qt_ver_f/single/qt-everywhere-src-$qt_ver_f.tar.xz \
    && tar -xJf qt-everywhere-src-$qt_ver_f.tar.xz && rm qt-everywhere-src-$qt_ver_f.tar.xz \
    && cd /qt/qt-everywhere-src-$qt_ver_f && ./configure -v -platform linux-g++ -static -release -prefix /qt -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg \
        -fontconfig -system-freetype -egl -opengl es2 -xcb -xcb-xlib -no-openssl -opensource -confirm-license -make libs -nomake tools -nomake examples -nomake tests \
    && make -s -j`nproc` && make install \
    && rm -rfd /qt/qt-everywhere-src-$qt_ver_f \
    && apt autoremove -y && apt autoclean -y \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


ENV PATH /qt/bin:$PATH

CMD ["/bin/sh"]
