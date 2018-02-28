FROM mageia:6
ARG arch=x86_64
ARG version=6
ARG username=testuser

# Install minimal tools
RUN urpmi.addmedia --distrib --mirrorlist "http://mirrors.mageia.org/api/mageia.${version}.${arch}.list"
RUN urpmi sudo vim-enhanced

# Avoid doing too much stuff as root, use a separate user instead
RUN useradd --create-home --shell /bin/bash --user-group --groups wheel ${username} \
    && echo '%wheel ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Switch to unprivileged user
USER ${username}
ENV HOME=/home/${username}
WORKDIR $HOME

# Allow user-specific binaries (like those installed with 'pip install --user' to be found
ENV PATH="${HOME}/.local/bin:${PATH}"
RUN sudo urpmi python3-pip
RUN pip3 install --upgrade pip \
    && pip3 install --user meson

# Install development packages
RUN sudo urpmi ninja
#RUN urpmi gcc pkgconfig ninja
#RUN urpmi libstdc++-devel lib64gtk+3.0-devel
