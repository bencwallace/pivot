FROM ubuntu:24.04

RUN apt update && apt install -y \
    clang \
    cmake \
    g++ \
    gdb \
    git \
    libboost-dev \
    libgraphviz-dev \
    pkg-config
