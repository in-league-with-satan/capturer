FROM ubuntu:20.04

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y libtool libtool-bin build-essential cmake mc curl \
        autoconf automake autopoint bash bison bzip2 flex g++ g++-multilib gettext git gperf \
        intltool libc6-dev-i386 libgdk-pixbuf2.0-dev libltdl-dev libssl-dev \
        libxml-parser-perl lzip make openssl p7zip-full patch perl pkg-config python python-mako \
        ruby sed unzip wget xz-utils \
    && git clone https://github.com/mxe/mxe.git && cd mxe && git checkout 3e54b1da61770ecaff799e8e0605aa937716d672 && make -j`nproc` MXE_TARGETS='x86_64-w64-mingw32.static.posix' MXE_PLUGIN_DIRS='plugins/gcc10' qtbase qtmultimedia qtquickcontrols qtquickcontrols2 qtdeclarative qtgraphicaleffects qtimageformats qtsvg qtwinextras \
    && apt-get purge -y --allow-remove-essential autopoint bison bzip2 flex g++ g++-multilib gettext gperf intltool libc6-dev-i386 libgdk-pixbuf2.0-dev \
        libltdl-dev libssl-dev libtool-bin libxml-parser-perl lzip patch ruby unzip wget xz-utils \
    && apt-get autoremove -y && apt-get autoclean -y \
    && rm -rfd /mxe/.ccache /mxe/.git /mxe/log /mxe/pkg /mxe/src /mxe/usr/x86_64-w64-mingw32.static.posix/bin \
        /mxe/usr/x86_64-pc-linux-gnu/doc /mxe/usr/x86_64-pc-linux-gnu/icu4c /mxe/usr/x86_64-pc-linux-gnu/include /mxe/usr/x86_64-pc-linux-gnu/lib /mxe/usr/x86_64-pc-linux-gnu/share \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN ln -s /mxe/usr/x86_64-w64-mingw32.static.posix/qt5/bin/qmake /mxe/usr/bin/qmake && \
    ln -s /mxe/usr/bin/x86_64-w64-mingw32.static.posix-g++ /mxe/usr/bin/g++

RUN chmod -R a+w /mxe

ENV PATH /mxe/usr/bin:$PATH
CMD ["/bin/sh"]
