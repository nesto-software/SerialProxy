# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.177.0/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="buster"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}
LABEL org.opencontainers.image.source https://github.com/nesto-software/SerialProxy

ENV TOOLCHAIN_RELEASE=rpi-5.10.y

ADD https://raw.githubusercontent.com/nesto-software/cross-toolchain-armhf/master/scripts/install-from-release.sh \
    /tmp/install-cross-toolchain-armhf.sh $TOOLCHAIN_RELEASE

RUN chmod +x /tmp/install-cross-toolchain-armhf.sh \
    && /tmp/install-cross-toolchain-armhf.sh

ENV STAGING_DIR /usr/raspberry-build/staging
ENV TOOLCHAIN=arm-unknown-linux-gnueabi
ENV TOOLCHAIN_PATH=/opt/crosstool-ng/x-tools/${TOOLCHAIN}/bin
ENV PATH=${TOOLCHAIN_PATH}:${PATH}

RUN mkdir -p "${STAGING_DIR}"

RUN sudo apt-get update -y && apt-get install -y libtool autoconf automake g++

# install libzmq
RUN git clone https://github.com/zeromq/libzmq.git /tmp/libzmq-2 && cd /tmp/libzmq-2 && git checkout 92282785ed8e3a954d379a0ac0e784dc29d94746
RUN apt-get install -y pkg-config

WORKDIR /tmp/libzmq-2/
RUN ./autogen.sh
# note: we must use both prefix options here because DESTPATH for make install places the files correctly but sets wrong pkg-config contents
RUN CC=${TOOLCHAIN}-gcc CXX=${TOOLCHAIN}-g++ AR=${TOOLCHAIN}-ar STRIP=${TOOLCHAIN}-strip RANLIB=${TOOLCHAIN}-ranlib ./configure --host=arm-none-linux-gnueabi --exec-prefix=$STAGING_DIR/usr/local --prefix=${STAGING_DIR}/usr/local --disable-curve-keygen 
RUN make
RUN make install

# install the CMAKE_TOOLCHAIN_FILE for our toolchain
ENV TOOLCHAIN_FILE /usr/raspberry-build/tmp/raspberry_pi_3_b_plus.cmake.tc
RUN mkdir -p /usr/raspberry-build/tmp/
COPY assets/armhf-cross-toolchain.cmake.tc $TOOLCHAIN_FILE

# install cppzmq
RUN git clone https://github.com/zeromq/cppzmq.git /tmp/cppzmq-2 && cd /tmp/cppzmq-2 && git checkout c591113bb7975e1be6fa6b0c758cacfe0411c66e

WORKDIR /tmp/cppzmq-2
RUN mkdir build
WORKDIR ./build

# install latest cmake binary directly (reason: cppzmq needs a feature of cmake 3.7+ regarding tarball name extraction)
RUN wget https://github.com/Kitware/CMake/releases/download/v3.19.4/cmake-3.19.4-Linux-x86_64.sh -P /tmp/
RUN chmod +x /tmp/cmake-3.19.4-Linux-x86_64.sh && mkdir /tmp/cmake && /tmp/cmake-3.19.4-Linux-x86_64.sh --skip-license --prefix=/tmp/cmake

# note: confirm pkg-content with pkg-config cli if in doubt: PKG_CONFIG_PATH=/usr/raspberry-build/staging/usr/local/lib/pkgconfig pkg-config libzmq --exists; echo $?
# note: without setting CMAKE_FIND_ROOT_PATH, the cmake find_* methods do not work as expected because of the toolchain file setting for CMAKE_FIND_ROOT_PATH_MODE_LIBRARY
RUN  CC=${TOOLCHAIN}-gcc CXX=${TOOLCHAIN}-g++ PKG_CONFIG_PATH=$STAGING_DIR/usr/local/lib/pkgconfig /tmp/cmake/bin/cmake .. "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE" "-DCMAKE_FIND_ROOT_PATH=$STAGING_DIR"
RUN  CC=${TOOLCHAIN}-gcc CXX=${TOOLCHAIN}-g++ make
RUN DESTDIR=${STAGING_DIR} make -j4 install

# aws-greengrass-sdk-c
RUN git clone https://github.com/aws/aws-greengrass-core-sdk-c.git /tmp/aws-sdk
RUN cd /tmp/aws-sdk/ && git checkout a71613711438b48588e177f86ab322dd3992e780 && cd ./aws-greengrass-core-sdk-c && mkdir -p build && cd build && cmake "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE" "-DCMAKE_FIND_ROOT_PATH=$STAGING_DIR" .. && cmake "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE" "-DCMAKE_FIND_ROOT_PATH=$STAGING_DIR" --build . && sudo make install DESTDIR="${STAGING_DIR}"
