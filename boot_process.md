# Boot Process Notes

1. i.MX Boot Process (NXP i.MX SoCs)
Key Stages
1. Boot ROM (on-chip, hardware-hardcoded)
• Executes immediately after power-on reset or system reset.
• Reads boot-mode selectors (fuses, strap pins) to determine
boot media (eMMC, SD card, SPI-NOR, NAND, USB-SDP, etc.).
(NXP Community)
• Initializes on-chip RAM (OCRAM/Tightly Coupled Memory) and
minimal infrastructure.
• Loads the next stage bootloader (typically SPL or direct U-
Boot) from the selected boot device.
• Important note: On modern i.MX8/i.MX9, the Boot ROM may
first load a “container” image which includes Firmware for the
System Controller (SCFW), Security Controller (SECO) and
others. (NXP Community)
2. Secondary Bootloader (SPL → full U-Boot) / U-Boot
• If SPL (Secondary Program Loader) is used: SPL runs in
OCRAM and performs early hardware init (e.g., DDR controller,
clocks) then loads full U-Boot into DDR.
• If no SPL, Boot ROM may load full U-Boot directly (depending
on SoC).
• U-Boot then:
• further configures memory, clocks, board hardware
• loads the Linux kernel image and the Device Tree Blob
(DTB) (and optionally initrd/initramfs).
• passes boot arguments (bootargs) and DTB to the
kernel.• may offer a console shell, boot prompt, networking, fast-
boot mode, recovery mode.
• Important note: On i.MX family, modern features like “Falcon
Mode” are supported to reduce boot time by skipping full U-
Boot and going direct to kernel. (NXP Semiconductors)
3. Linux Kernel Execution
• U-Boot loads the kernel (zImage/Image) into RAM, sets up
DTB and bootargs, then jumps into kernel entry point.
• Kernel uncompresses, initializes CPU(s), memory
management, device drivers, peripheral initialization.
• The Device Tree describes hardware layout (so kernel can bind
drivers properly).
• Kernel mounts the root filesystem (or an initramfs/initrd) and
transitions to user-space.
• Important note: For secure boot, trusted firmware (e.g., ARM
TF-A), SECO, SCFW must be loaded prior to kernel on many
i.MX8/9 devices. (NXP Community)
4. Root Filesystem & Init Process
• Root filesystem (e.g., ext4, SquashFS, UBIFS, or network root)
is mounted.
• The init system (either systemd, busybox init, or other) starts
/sbin/init (or equivalent).
• System services are brought up, user-space processes start.
Additional Important Notes
• Boot Time Optimization: Application notes from NXP describe
techniques (boot delay removal, Falcon mode, kernel command line
tweaks) to reduce total boot time. (NXP Semiconductors)• Boot Media Fallback: If the selected boot media fails (e.g., no valid
image), the Boot ROM may fallback to Serial Download Protocol
(SDP) or alternate boot device. (NXP Community)
• Board/SoC Variations: Different i.MX series (6,7,8,9) embed different
subsystems (System Controller, Cortex-M domains, edge security)
so actual boot steps may include additional sub-steps (e.g., SCFW,
Cortex-M core boot).
2. Linux Boot Process (General-Purpose Systems,
e.g., x86)
Key Stages
1. BIOS / UEFI Initialization
• On power-on, the BIOS/UEFI firmware performs POST (Power-
On Self-Test), initializes basic hardware (CPU, RAM, chipset).
• Detects bootable devices (HDD, SSD, USB, network) and loads
the bootloader from selected media (MBR, EFI partition).
• On some embedded systems, BIOS/UEFI is replaced or
omitted altogether; the SoC boot-ROM directly loads an
embedded bootloader.
2. Bootloader (e.g., GRUB / LILO / Syslinux)
• Presents menu, allows parameter selection.
• Loads the Linux kernel image (vmlinuz) and optionally an
initial RAM disk (initrd/initramfs).
• Passes kernel parameters (root filesystem location, options)
and may load Device Tree on non-x86 architectures.
• Transfers control to the kernel.
3. Linux Kernel Initialization• Kernel decompresses/unpacks itself, initializes system
memory, scheduler, device drivers, peripheral subsystems.
• Mounts initrd or root filesystem as specified.
• Kernel then executes the user-space init process.
4. Init / systemd & User Space Startup
• The init system (/sbin/init -> systemd or SysV) starts up system
services (networking, login managers, GUIs).
• GUI (Xorg, Wayland, desktop environment) may load
depending on system type.
• User applications launch and system becomes ready for
interaction.
Additional Important Notes
• On embedded variants of Linux, you might skip BIOS/UEFI and use
U-Boot or other bootloader directly (see next section).
• Init systems are evolving: systemd is now dominant on many
distributions.
• Root filesystem may reside locally (SSD, HDD) or be network-
mounted (NFS) depending on target deployment.
• For secure boot or measured boot on PC/servers, UEFI Secure Boot
adds steps of signature verification (not covered in embedded i.MX
case).
3. ARM Boot Process (General ARM-based
Embedded Devices)
Key Stages
1. Boot ROM (on-chip in SoC)
• After power‐on reset, the ARM core begins execution from a
fixed address in ROM.• Boot-ROM code initializes basic hardware and selects boot
media based on straps/fuses.
• Loads the next stage bootloader (could be U-Boot, Barebox,
Little Kernel, etc.).
2. Bootloader (e.g., U-Boot / Barebox / LK)
• Initialize DRAM/external memory, set up clocks, UART,
peripheral controllers.
• Load kernel + DTB + (optionally) initrd, pass arguments.
• May provide UI shell, fastboot/USB, recovery.
3. Linux Kernel Execution
• Same as above: decompress, init devices, mount root
filesystem.
• Kernel takes care of platform-specific drivers via DTB.
4. System Initialization (init/systemd)
• Starts system services, daemons, applications.
• Load optional GUI or operate command-line only.
Additional Important Notes
• This process is essentially a more generic version of the i.MX
sequence when used with ARM-based boards like Raspberry Pi or
BeagleBone.
• There can be additional complexities: multi-core initialization,
secure/non-secure domains, secondary cores (Cortex-M)
initialization.
• Bootloader size limitations may force the use of SPL or minimal
preloader: Boot ROM → SPL → full bootloader → kernel (as with
i.MX).
• Device Tree is essential for hardware abstraction in many ARM
SoCs.4. Android Boot Process (Based on Linux Kernel,
Mobile/Embedded Devices)
Key Stages
1. Boot ROM (SoC specific)
• On power ON / reset: SoC Boot ROM reads boot mode straps,
initializes minimal hardware, selects boot device (eMMC, UFS,
SD, USB).
• Loads bootloader or image container (depending on SoC) into
memory.
2. Bootloader (Fastboot / U-Boot / Little Kernel / OEM Bootloader)
• Initializes basic hardware (memory, UART, power, clocks).
• Loads boot image (boot.img) which includes: Linux kernel +
ramdisk (for Android) + DTB.
• May also load recovery image, vendor image, device-
specific blobs.
• May offer fastboot mode, OEM unlock, recovery, flashing
interface.
3. Kernel and Init
• Kernel uncompresses, initializes hardware and drivers.
• Ramdisk executes init.rc or init.<board>.rc, creating mount
points for /system, /vendor, /data.
• Android-specific components such as SELinux enforcement,
Binder driver initialization run.
4. Zygote & Android Runtime (ART/Dalvik)
• The Zygote process starts (forks for each Android app) and
pre-loads core Java classes.• The Android Runtime (ART) gets initialized; native and Java
services start.
5. System Server & Services Start
• Android’s system_server starts services: WindowManager,
ActivityManager, PackageManager, PowerManager.
• Boot animation plays.
6. Applications & User Interaction
• Launcher/home screen appears; apps can be launched;
system is ready for user interaction.
Additional Important Notes
• On mobile devices, boot time and responsiveness are critical; many
vendors employ techniques like kernel/ramdisk optimizations and
minimal services at boot.
• Secure Boot, Verified Boot, API levels, bootloader unlocking are
major concerns in Android.
• The root filesystem layout is different: /system, /vendor, /boot,
/recovery, and user data partition—so the mount and init process is
tailored for Android.
6. Summary Table (All Boot Processes)
Stage
i.MX (NXP
SoC)
Linux
ARM
Android
Key Notes (2025
(x86/Server (Embedd
(AOSP-Based)
Updates)
/Desktop)
ed)
Boot ROM
BIOS /
Boot
(SoC
UEFI– POST, ROM–
Boot
internal)–
initializes
Minimal
ROM /
Initializes CPU CPU/RAM– setup
Firmware
& OCRAM–
Detects
(clock,
Detects boot disks– Loads SRAM)–
1️⃣
✅
UEFI
Boot ROM–
replaces legacy
Same as ARM– BIOS on most
Loads Fastboot x86 and ARM64
/ LK / Aboot
platforms.
Secure Boot /
✅Stage
i.MX (NXP
SoC)
Linux
ARM
Android
Key Notes (2025
(x86/Server (Embedd
(AOSP-Based)
Updates)
/Desktop)
ed)
media
(eMMC, SD,
Bootloader
NAND, QSPI)–
(GRUB)
Loads SPL or
U-Boot
Loads
first
stage
bootload
er
Verified Boot
widely enforced.
✅
ARM Trusted
SPL → U-
U-Boot / Fastboot /
Firmware (TF-A)
GRUB2 /
Boot– Init
Barebox Little Kernel now used for
systemd-
DDR, clocks,
/ TF-A– (LK)– Verifies secure boot in
boot– Loads
PMIC– Loads
DDR init, signatures
most ARM64
kernel +
Bootloade kernel + DTB
peripher (AVB)– Loads
SoCs.
initrd–
r
+ rootfs–
al bring- boot.img
Bootloader
Reads
(Primary) Passes
up–
(kernel +
partitions on
/boot/grub.c
bootargs via
Loads
ramdisk)–
Android follow
fg or EFI
ATAGS or
kernel + Passes control A/B update
vars
Device Tree
DTB
to kernel
scheme
(seamless OTA).
2️⃣
✅
Linux
Linux Kernel
Linux
Kernel
(v6.x)–
Kernel
(v6.x)–
Decompress
(v6.x)– Init Similar
& mount
device
flow as
Kernel rootfs–
drivers,
i.MX–
Initializati Initialize
filesystems– Board
on
drivers,
Mount initrd support
regulators,
/ rootfs–
via
clocks– Setup
Start PID 1 Device
/dev, /proc,
(systemd)
Tree
/sys
(DTB)
3️⃣
4️⃣
Init
System /
RootFS
/sbin/init /
systemd–
Mount FS–
Start
daemons–
✅
Generic
Android Linux Kernel Image
Kernel (GKI)– (GKI) unifies
Same base
Android kernel
kernel,
builds.
Device
modularized– Tree Overlays
Loads
(DTO) now
system.img,
common for
vendor.img–
modular
Starts init.rc
hardware
configs.
systemd– init /
Android init
Standard on busybox (init.rc)–
all major
/
Parses init.rc
distros–
systemd scripts– Starts
Starts
– Starts zygote,
✅
✅
systemd
dominates Linux
ecosystem.
Android uses its
own init for
✅Stage
i.MX (NXP
SoC)
Launch user
services
5️⃣
CLI or
User
Embedded
Space /
UI– Custom
Applicatio
apps or
n Layer
services
Linux
ARM
Android
Key Notes (2025
(x86/Server (Embedd
(AOSP-Based)
Updates)
/Desktop)
ed)
network,
login, UIembedd
ed
surfaceflinger
daemons
Desktop /
Server UI–
GNOME,
KDE, etc.Zygote
preloads
Zygote →
common classes
SystemServer
CLI or
→ faster app
→ Launcher–
minimal
startup.
Starts Android
GUI
Android uses
runtime (ART)–
Binder IPC +
Launches apps
SELinux
Enforcing.
precise startup
ordering.
✅
✅
✅
6️⃣
High
Security /
Assurance
Verificatio
Boot (HAB)
n
Secure boot
mandatory on
Secure
most 2025
Secure Boot Boot /
AVB (Android devices.
/ TPM 2.0
TrustZon Verified Boot) Verified Boot
e
checks every
stage (chain of
trust).
✅
Role of an Embedded Software Engineer in the
Boot Process
Boot Stage
1️⃣
Boot
ROM /
Firmware
Stage
System
Component
On-Chip
ROM (SoC
vendor-
provided)
🔹
Engineer’s Work /
Responsibilities
Analyze SoC boot
sequence and
supported boot modes
(SD, eMMC, QSPI,
NAND, USB)
🔹
Common Tools & Skills
➤ TRM / RM reading➤
Serial boot tools (e.g.,
imx_usb_loader,
fastboot)➤ NXP
MCUExpresso /Boot Stage
System
Component
Engineer’s Work /
Responsibilities
Common Tools & Skills
Configure fuses or OTP
bits for boot device
selection Study
STM32CubeProg / JTAG
reference manuals and tools
TRM to understand
boot flow
🔹
🔹
🔹
Port or customize
U-Boot for the board
Add board-specific
initialization (DDR
➤ U-Boot source (board/,
timing, PMIC, pinmux, include/configs/)➤
First and
clocks) Add
Cross-compilation
Bootloader
Second
environment variables (arm64-gcc)➤
(SPL / U-Boot / Stage
(bootargs, bootcmd)
fw_printenv, fw_setenv,
TF-A)
Bootloaders Enable drivers (I2C, SPI, printenv, mmc, loadb
UART, eMMC,
commands➤ JTAG /
Ethernet) Integrate UART debug
secure boot (HAB / TF-
A) Debug with serial
console
2️⃣
🔹
🔹
🔹
3️⃣
Kernel
Stage
4️⃣
Root
🔹
🔹
Board Support
Package (BSP) work:
• Add/modify Device
Tree (.dts/.dtsi) •
Integrate custom
➤ Linux kernel build
drivers (sensors, PMIC, system➤ menuconfig,
Linux Kernel
GPIO, I2C, SPI) •
make zImage, make
(v6.x or
Configure defconfig
dtbs➤ Device Tree
higher)
and enable kernel
editing➤ JTAG / serial
modules Optimize
logs
boot time (disable
unused drivers)
Debug kernel boot logs
(via dmesg, printk)
🔹
RootFS
🔹
🔹
Build and integrate
➤ Yocto Project (BitBake,Boot Stage
Filesystem &
Init Stage
System
Component
(Yocto /
Buildroot /
Debian)
Engineer’s Work /
Responsibilities
Common Tools & Skills
🔹
RootFS using Yocto /
Buildroot Add
custom startup scripts
(/etc/init.d/, systemd
units) Configure
recipes)➤ Buildroot➤
mount points,
BusyBox utilities➤
permissions, network, systemd configuration
and user-space
daemons Debug
early userspace failures
(init, systemd-analyze)
🔹
🔹
🔹
Develop or port
embedded applications
(C/C++/Python)
➤ C/C++ app
Interface with device
development➤
System
drivers via sysfs, ioctl,
User Space
POSIX/Linux APIs➤ IPC
Services &
or userspace libraries
/ Applications
(shared memory,
Applications Test end-to-end
sockets, DBus)➤ Git,
functionality (sensor
CI/CD, unit testing
data → user app)
Handle OTA updates,
A/B partitions
🔹
5️⃣
🔹
🔹
🔹
🔹
🔹
Implement Secure
Boot, HAB, AVB
Enable encryption (dm-
verity, LUKS, TEE)
➤ TF-A / OP-TEE➤
System
Security /
Optimize boot time
PowerTOP, systemd-
Hardening &
Optimization
(parallel init, deferred analyze➤ perf / ftrace /
Performance
probing) Power
powertop
management
(suspend/resume,
DVFS)
6️⃣
7️⃣
Debug &
Validation
🔹
🔹
Cross-
Bring-up hardware
System
(UART, DDR, I2C, SPI,
Integration GPIO tests) Use
🔹
➤ JTAG, OpenOCD➤
serial console logs➤
minicom / picocom➤Boot Stage
System
Component
Engineer’s Work /
Responsibilities
Common Tools & Skills
oscilloscope, logic
analyzer for signal-level
debug Kernel crash /
GDB cross-debugging
panic analysis Root
cause analysis for boot
hangs
🔹 🔹
🧠
Simplified Summary
Boot Layer
Main Engineer Focus
Boot ROMUnderstand boot device order & secure fuse
setup
BootloaderBoard bring-up (DDR, PMIC, peripherals)
KernelAdd device drivers, edit Device Tree
RootFSBuild system image (Yocto/Buildroot)
Init SystemCustomize service startup (systemd / init.d)
Application
LayerDevelop and test user-space applications
SecurityEnable Secure Boot, encryption, TrustZone
DebugAnalyze boot logs, fix hangs, test full boot
cycle
Example (Typical Embedded Project Flow)
[Board Power On]
↓
[Boot ROM - Vendor]
↓
[Bootloader (U-Boot) - Engineer customizes board init]
↓
[Kernel (v6.x) - Engineer adds drivers & Device Tree]
↓
[RootFS - Engineer builds with Yocto]↓
[Init (systemd) - Engineer tunes startup & scripts]
↓
[Applications - Engineer develops/testing user logic]
Real Example: i.MX8M Board Bring-up
Task
Engineer Action
DDR not initializingModify DDR timing in board.c
UART missing logsEnable UART node in .dts
Sensor not detectedWrite/port I2C driver in /drivers/iio/
Kernel boot stopsDebug with earlycon or JTAG
RootFS not mountingFix /etc/fstab or bootargs
Long boot timeUse systemd-analyze blame
Secure BootUse HAB tools to sign U-Boot image
