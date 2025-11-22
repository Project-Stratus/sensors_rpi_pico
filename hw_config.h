#pragma once
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"

size_t sd_get_num();
sd_card_t *sd_get_by_num(size_t num);
