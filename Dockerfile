FROM ubuntu:24.04 

RUN apt-get -qy update && apt-get -qy install build-essential clang llvm libbpf-dev iproute2 linux-libc-dev linux-headers-$(uname -r) gcc-multilib
RUN ln -s /usr/include/x86_64-linux-gnu/asm /usr/include/asm

WORKDIR /src
COPY ebpf-clat.c ebpf-clat.c
RUN clang -O2 -target bpf -c ebpf-clat.c -o clat.o
