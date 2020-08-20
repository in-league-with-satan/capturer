FROM ubuntu:20.04

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y libtool build-essential cmake mc curl \
        autoconf automake autopoint bash bison bzip2 flex g++ g++-multilib gettext git gperf \
        intltool libc6-dev-i386 libgdk-pixbuf2.0-dev libltdl-dev libssl-dev libtool-bin \
        libxml-parser-perl lzip make openssl p7zip-full patch perl pkg-config python \
        ruby sed unzip wget xz-utils \
    && git clone --depth 1 https://github.com/mxe/mxe.git && cd mxe && make -j`nproc` MXE_TARGETS='x86_64-w64-mingw32.static.posix' MXE_PLUGIN_DIRS='plugins/gcc10' qtbase qtmultimedia qtquickcontrols2 qtwinextras \
    && apt-get purge -y --allow-remove-essential libtool cmake pkg-config autoconf automake autopoint bison bzip2 flex g++ g++-multilib gettext gperf intltool libc6-dev-i386 libgdk-pixbuf2.0-dev \
        libltdl-dev libssl-dev libtool-bin libxml-parser-perl lzip patch pkg-config ruby sed unzip wget xz-utils \
    && apt-get autoremove -y && apt-get autoclean -y \
    && rm -rfd /mxe/log /mxe/pkg /mxe/src /mxe/usr/x86_64-pc-linux-gnu /mxe/usr/x86_64-w64-mingw32.static.posix/bin \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN ln -s /mxe/usr/x86_64-w64-mingw32.static.posix/qt5/bin/qmake /mxe/usr/bin/qmake && \
    ln -s /mxe/usr/bin/x86_64-w64-mingw32.static.posix-g++ /mxe/usr/bin/g++

ENV PATH /mxe/usr/bin:$PATH
CMD ["/bin/sh"]
