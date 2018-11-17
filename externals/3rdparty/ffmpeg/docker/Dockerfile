#FROM ubuntu:18.04
FROM ubuntu:latest

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y autoconf automake libtool build-essential cmake git pkg-config libva-dev \
    gcc-8 g++-8 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8 && \
    update-alternatives --config gcc \
    && apt-get autoremove -y && apt-get autoclean -y \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

CMD ["/bin/bash"]
