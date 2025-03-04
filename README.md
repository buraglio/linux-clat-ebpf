# Reference ebpf-clat

# How to Compile and Deploy the eBPF CLAT Program

Compiling this *should* yield an simple reference implementation of eBPF XDP program that translates IPv4 packets into IPv6 packets using NAT64, in compliance with RFC 6877.

## Prerequisites

The following is required:

- A Linux system with eBPF and XDP support (Linux kernel 5.x or later is recommended).
- Clang and LLVM installed for compiling eBPF programs.
- `iproute2` package installed for managing XDP programs.
- `libbpf` development libraries installed.
- A PLAT (a NAT64 system upstream)

Most modern linux systems should work, I think.

## Compilation Steps

1. **Install Required Dependencies**
   ```sh
   sudo apt update
   sudo apt install -y clang llvm libbpf-dev iproute2
   ```

2. **Compile the eBPF Program**
   ```sh
   clang -O2 -target bpf -c ebpf-clat.c -o clat.o
   ```

## Deploying the eBPF Program (assuming eth0)

1. **Attach the eBPF Program to an Interface - NEEDS MORE TESTING -**
   ```sh
   ip link set dev eth0 xdp obj clat.o
   ```
   Replace `eth0` with the correct network interface for your system.

2. **Verify that the Program is Running**
   ```sh
   ip -s link show dev eth0
   ```
   Look for XDP statistics to confirm the program is attached and processing packets.

## Unloading the eBPF Program

If you need to remove the eBPF program, run:
   ```sh
   ip link set dev eth0 xdp off
   ```

## Debugging and Logs

- Use `dmesg` to check for kernel logs in case of failures.
  ```sh
  dmesg | tail
  ```
- Use `bpftool` to inspect loaded eBPF programs:
  ```sh
  bpftool prog show
  ```

## Notes
- Ensure that IPv6 is enabled on the system (duh).
- Modify the NAT64 prefix if using a custom prefix, this one assumes the standard.
- Test functionality using `tcpdump` or Wireshark or tshark, or something else to verify packet translations.
- This is nothing more than a test, a reference, if you will. It may not even work for you and I probably can't fix any problems you have with it.
