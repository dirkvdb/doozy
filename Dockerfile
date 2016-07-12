FROM base/archlinux
MAINTAINER Dirk Vanden Boer

RUN pacman-key --populate && \
  pacman-key --refresh-keys && \
  pacman -Sy --noprogressbar --noconfirm && \
  pacman -S archlinux-keyring pacman --noprogressbar --noconfirm && \
  pacman-db-upgrade && \
  pacman -Syyu --noprogressbar --noconfirm && \
  pacman -S gcc yasm taglib openal alsa-lib libjpeg-turbo libpng ffmpeg cmake autoconf automake ninja libtool pkg-config make patch flac libmad libunwind libxdg-basedir zlib sqlite libcec git --noprogressbar --noconfirm