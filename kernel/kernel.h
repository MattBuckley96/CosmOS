#pragma once

#include "types.h"
#include "console.h"
#include "util.h"

#include "cpu/io.h"
#include "cpu/idt.h"

#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "drivers/ata.h"

#include "shell/shell.h"

void panic(const u8* msg);
