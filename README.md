# stm32 playground (bluepill)

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

The build prints a size summary via `arm-none-eabi-size`.

### 3. Flash

**Via ST-Link:**

```sh
make flash
```

**Via dapboot USB DFU:**

1. Put the board into DFU mode (hold BOOT, tap RESET).
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

- MCU: `STM32F103xB`, Cortex-M3, 64 KB flash / 20 KB RAM
- HSE: 8 MHz crystal → PLL ×9 → **72 MHz** SYSCLK
- User LED: **PC13**, active-low (on when pin is LOW)
- Button (not currently used): PA0, active-low with internal pull-up

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

### Update to latest upstream

```sh
git submodule update --remote --merge
git add HAL/STM32CubeF1 RTOS/FreeRTOS-Kernel bootloader/dapboot
git commit -m "Bump submodules"
```

### Make a local change inside a submodule

Submodules track a **commit hash**, not files. If you need to patch a submodule, fork it, push your changes there, then point the submodule at your fork:

```sh
cd RTOS/FreeRTOS-Kernel
git remote add fork git@github.com:<you>/FreeRTOS-Kernel.git
git push fork HEAD:main
cd ../..
git submodule set-url RTOS/FreeRTOS-Kernel git@github.com:<you>/FreeRTOS-Kernel.git
git add .gitmodules RTOS/FreeRTOS-Kernel
git commit -m "Point FreeRTOS-Kernel at fork"
```

## Firmware notes

### `HAL_Delay()` and `HAL_IncTick()`

`HAL_Delay()` blocks until the SysTick interrupt has advanced `HAL_GetTick()` by the requested number of milliseconds. The SysTick handler must call `HAL_IncTick()`:

```c
void SysTick_Handler(void) {
    HAL_IncTick();
}
```

If you remove that call, `HAL_Delay()` hangs forever. When you later start FreeRTOS, FreeRTOS owns SysTick and you'll need to restructure this — see below.

### FreeRTOS

The FreeRTOS kernel sources are compiled but `vTaskStartScheduler()` is **not** called yet. Unused code is stripped by `--gc-sections`, so this costs nothing in flash. When you do start the scheduler, also add to `FreeRTOSConfig.h`:

```c
#define vPortSVCHandler      SVC_Handler
#define xPortPendSVHandler   PendSV_Handler
#define xPortSysTickHandler  SysTick_Handler
```

and move `HAL_IncTick()` out of `SysTick_Handler` (FreeRTOS replaces it).

## Simulation and tests

Host-side tooling lives in `sim/` and `tests/`. These build against stub HAL headers so you can exercise pure logic (debounce, state machines, protocol parsers) on your PC without flashing anything:

```sh
make -C tests
```

(Not implemented yet — see git history for plans.)

## Suggested learning path

Each step is a single commit:

1. **Blink** — `HAL_GPIO_TogglePin` + `HAL_Delay`. *(current state)*
2. **Non-blocking blink** — replace `HAL_Delay` with `HAL_GetTick()` arithmetic.
3. **Button → LED** — read PA0, debounce with a shift-register filter.
4. **UART `printf`** — enable USART1, retarget `_write`.
5. **FreeRTOS tasks** — two tasks, one blinks, one prints.
6. **dapboot** — flash the bootloader once, update firmware over USB DFU.

## License

Application code (`Core/`, `Makefile`, etc.): **MIT** — add a `LICENSE` file.

Submodules retain their own licenses:

| Submodule | License |
|-----------|---------|
| STM32CubeF1 | BSD-3-Clause (STMicroelectronics) |
| FreeRTOS-Kernel | MIT |
| dapboot | MIT |
