#FROM fedora:latest
FROM ubuntu:latest


#RUN dnf install -y gcc-c++ make cmake autoconf automake libtool git-core pkg-config wget xz which python \
#        freetype-devel fontconfig-devel pulseaudio-libs-devel libva-devel p7zip p7zip-plugins \
#        mesa-libGL-devel libXi-devel libXcomposite libXcursor-devel libXtst-devel libxkbcommon-devel libxkbcommon-x11-devel \
#        mesa-libGLES-devel egl-wayland-devel mesa-libEGL-devel libglvnd-egl ncurses-devel \
#    && mkdir qt && cd qt \
#    && export qt_ver_f=`git ls-remote --tags https://github.com/qt/qt5 | awk '{print $2}' | grep -v '\^{}$' | grep -v '-' | sort -V | tail -1 | awk '{split($0, a, "/"); print a[3]}' | awk '{split($0, a, "v"); print a[2]}'` \
#    ; export qt_ver_s=`echo $qt_ver_f | awk '{split($0, a, "."); printf("%s.%s", a[1], a[2])}'` \
#    ; wget http://download.qt.io/archive/qt/$qt_ver_s/$qt_ver_f/single/qt-everywhere-src-$qt_ver_f.tar.xz \
#    && tar -xJf qt-everywhere-src-$qt_ver_f.tar.xz && rm qt-everywhere-src-$qt_ver_f.tar.xz \
#    && cd /qt/qt-everywhere-src-$qt_ver_f && ./configure -v -platform linux-g++ -static -release -prefix /qt -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg \
#        -fontconfig -system-freetype -egl -opengl es2 -no-openssl -opensource -confirm-license -make libs -nomake tools -nomake examples -nomake tests \
#    && make -j`nproc` && make install \
#    && rm -rfd /qt/qt-everywhere-src-$qt_ver_f \
#    && dnf remove -y python && dnf clean all


RUN apt update && apt install -y gcc-8 g++-8 make cmake autoconf automake libtool git pkg-config wget xz-utils libfreetype6-dev libfontconfig1-dev libpulse-dev libva-dev p7zip-full python \
        mesa-common-dev libgl1-mesa-dev libxi-dev libxcomposite-dev libxcursor-dev libxtst-dev libxkbcommon-dev libxkbcommon-x11-dev \
        libgles2-mesa-dev libwayland-egl-backend-dev libgles2-mesa-dev libncurses5-dev libncursesw5-dev \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8 \
    && update-alternatives --config gcc \
    && mkdir /qt && cd /qt \
    && export qt_ver_f=`git ls-remote --tags https://github.com/qt/qt5 | awk '{print $2}' | grep -v '\^{}$' | grep -v '-' | sort -V | tail -1 | awk '{split($0, a, "/"); print a[3]}' | awk '{split($0, a, "v"); print a[2]}'` \
    ; export qt_ver_s=`echo $qt_ver_f | awk '{split($0, a, "."); printf("%s.%s", a[1], a[2])}'` \
    ; wget http://download.qt.io/archive/qt/$qt_ver_s/$qt_ver_f/single/qt-everywhere-src-$qt_ver_f.tar.xz \
    && tar -xJf qt-everywhere-src-$qt_ver_f.tar.xz && rm qt-everywhere-src-$qt_ver_f.tar.xz \
    && cd /qt/qt-everywhere-src-$qt_ver_f && ./configure -v -platform linux-g++ -static -release -prefix /qt -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg \
        -fontconfig -system-freetype -egl -opengl es2 -no-openssl -opensource -confirm-license -make libs -nomake tools -nomake examples -nomake tests \
    && make -j`nproc` && make install \
    && rm -rfd /qt/qt-everywhere-src-$qt_ver_f \
    && apt autoremove -y && apt autoclean -y \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


ENV PATH /qt/bin:$PATH

CMD ["/bin/sh"]
