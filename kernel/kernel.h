#pragma once

#include "types.h"
#include "console.h"
#include "cpu/io.h"
#include "util.h"
#include "cpu/idt.h"
#include "drivers/keyboard.h"

void panic(const u8* msg);
