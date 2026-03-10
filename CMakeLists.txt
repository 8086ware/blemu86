cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)

project(blemu86)

add_executable(blemu86 "src/main.cpp" "src/86blemu.h" "src/cpu/cpu.h" "src/chipset/bus.h" "src/chipset/bus.cpp"  "src/device.h" "src/cga.h" "src/chipset/pit.h" "src/cga.cpp" "src/chipset/pit.cpp"  "src/chipset/pic.h" "src/chipset/pic.cpp" "src/cpu/cpu.cpp"  "src/to_linear.cpp" "src/to_linear.h"  "src/86blemu.cpp"  "src/crtc.cpp" "src/chipset/ppi.h" "src/chipset/ppi.cpp" "src/keyboard.cpp" "src/keyboard.h"  "src/floppy.h" "src/floppy.cpp" "src/bios_rom.h" "src/chipset/dma.h" "src/chipset/dma.cpp" "src/cpu/exec.cpp"  "src/cpu/modrm.cpp" "src/cpu/operand.cpp" "src/cpu/instruction.cpp" "src/cpu/flags.cpp")

target_include_directories(blemu86 PUBLIC "src")

find_package(SDL3 REQUIRED)

target_link_libraries(blemu86 PUBLIC SDL3::SDL3)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)