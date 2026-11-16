# stm32 playground

A minimal STM32F1 playground built **without STM32CubeMX**, just a plain `Makefile`.
Currently contains a blink example: PC13 toggles at 1 Hz.

## Features

- Single `make` build
- STM32CubeF1 HAL and FreeRTOS-Kernel pulled in as submodules
- dapboot USB DFU bootloader available as a submodule
- Reproducible from a fresh clone (`git clone --recurse-submodules`)

## Repository layout

```
.
├── Core/
│   ├── inc/                  # Application headers (main.h, …)
│   └── src/                  # Application sources (main.c, stm32f1xx_it.c, …)
├── HAL/STM32CubeF1/          # STM32CubeF1 (submodule)
├── RTOS/FreeRTOS-Kernel/     # FreeRTOS kernel (submodule)
├── bootloader/dapboot/       # dapboot USB DFU bootloader (submodule)
├── linker/                   # Linker scripts
├── startup/                  # Startup assembly (if not using CMSIS copy)
├── lib/                      # Third-party libraries
├── sim/                      #  simulation harness 
├── tests/                    #  unit tests 
├── Makefile
└── README.md
```

## Requirements

| Tool | Purpose |
|------|---------|
| `arm-none-eabi-gcc` | ARM Cortex-M toolchain |
| `make` | Build system |
| `git` | Submodules |
| `st-flash` or `openocd` | Flashing via ST-Link |
| `dfu-util` | Flashing via dapboot USB DFU (optional) |

Install on Void Linux:

```sh
sudo xbps-install -S base-devel cross-arm-none-eabi openocd dfu-util git
```

Install on Debian/Ubuntu:

```sh
sudo apt install build-essential gcc-arm-none-eabi openocd dfu-util git
```

## Getting started

### 1. Clone (with submodules)

```sh
git clone --recurse-submodules https://github.com/<you>/blink_hal.git
cd blink_hal
```

Already cloned without submodules?

```sh
git submodule update --init --recursive
```

### 2. Build

```sh
make
```

Artifacts:

| File | Description |
|------|-------------|
| `build/firmware.elf` | ELF with debug symbols |
| `build/firmware.bin` | Raw binary (for DFU / ST-Link) |
| `build/firmware.hex` | Intel HEX |
| `build/firmware.map` | Linker map |

### 3. Flash

**Via ST-Link:**

```sh
make flash
```

**Via dapboot USB DFU:**

1. Put the board into DFU mode (BOOT1 = 1, tap RESET).
2. Flash:

```sh
dfu-util -d 1d50:6018 -a 0 -s 0x08000000:leave -D build/firmware.bin
```

### 4. Clean

```sh
make clean
```

## Target board

Defaults assume an **STM32F103C8 "Blue Pill"**:
If your board differs, adjust:

- `LED_GPIO_PORT` / `LED_GPIO_PIN` in `Core/inc/main.h`
- Clock PLL multiplier in `SystemClock_Config()` (`Core/src/main.c`)
- Linker script in `Makefile` (`LDSCRIPT`)
- `DEFINES := -DSTM32F103xB` in `Makefile`

## Configuration

| What | Where |
|------|-------|
| Target MCU / CPU flags | `Makefile` (`CPU`, `DEFINES`) |
| Linker script | `Makefile` (`LDSCRIPT`) |
| HAL sources compiled in | `Makefile` (`C_SOURCES`) |
| FreeRTOS sources compiled in | `Makefile` (`FREERTOS_FILES`) |
| Include paths | `Makefile` (`INCLUDES`, `CFLAGS`) |
| LED / button pins | `Core/inc/main.h` |
| Clock tree | `SystemClock_Config()` in `Core/src/main.c` |

HAL modules can be enabled individually by uncommenting the relevant lines in the `C_SOURCES` block. Only the modules you actually use get compiled.

## Submodules

| Submodule | Upstream |
|-----------|----------|
| `HAL/STM32CubeF1` | [STMicroelectronics/STM32CubeF1](https://github.com/STMicroelectronics/STM32CubeF1) |
| `RTOS/FreeRTOS-Kernel` | [FreeRTOS/FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) |
| `bootloader/dapboot` | [devanlai/dapboot](https://github.com/devanlai/dapboot) |

## License

Application code (`Core/`, `Makefile`, etc.): **MIT**

Submodules retain their own licenses:

| Submodule | License |
|-----------|---------|
| STM32CubeF1 | BSD-3-Clause (STMicroelectronics) |
| FreeRTOS-Kernel | MIT |
| dapboot | MIT |
