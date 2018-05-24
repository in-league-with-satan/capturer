FROM ubuntu:18.04

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y autoconf automake libtool build-essential cmake git subversion mercurial pkg-config \
    && apt-get autoremove -y && apt-get autoclean -y

CMD ["/bin/bash"]
