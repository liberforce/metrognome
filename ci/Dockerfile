FROM mageia:6
ARG arch=x86_64
ARG version=6

RUN urpmi.addmedia --distrib --mirrorlist "http://mirrors.mageia.org/api/mageia.${version}.${arch}.list"
RUN urpmi gcc pkgconfig python3-pip libstdc++-devel lib64gtk+3.0-devel ninja
RUN pip3 install --upgrade pip \
    && pip3 install meson
