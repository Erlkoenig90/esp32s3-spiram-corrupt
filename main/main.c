#include <stdint.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"
#include "esp_private/esp_clk.h"

// #include "rom/crc.h"

static const char * const TAG = "main";

uint32_t fill (uint8_t* mem, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		mem [i] = i & 0xFF;
	}

	return 0; // crc32_le (0, mem, size);
}

bool check (uint8_t volatile* mem, size_t size, uint32_t chksum, const char* label) {
/*	uint32_t got = crc32_le (0, mem, size);
	if (got != chksum) {
		ESP_LOGE (TAG, "Memory checksum mismatch, got=%08" PRIx32 ", expect=%08" PRIx32 ", label=%s", got, chksum, label);
		abort ();
	} */
	(void) chksum;
	bool ok = true;

	for (size_t i = 0; i < size; ++i) {

		uint8_t prev = mem [i];
		size_t mismatches = 0;
		for (size_t j = 0; j < 5; ++j) {
			if (mem [i] != prev)
				++mismatches;
		}

		if (mismatches != 0 || prev != (i & 0xFF)) {
			ESP_LOGE (TAG, "Memory check failed at addr=%p i=%7zu, got=%02" PRIx8 ", expect=%02" PRIx8 ", xor=%02" PRIx8 ", mismatches=%zu, label=%s", mem+i, i, prev, (uint8_t) (i&0xFF), (uint8_t) ((i&0xFF) ^ prev), mismatches, label);
//			abort ();
			ok = false;
		}
	}
	return ok;
}

static const size_t bufSize = 1024*1024*7;

void app_main (void) {
#ifdef CONFIG_PM_ENABLE
    // Similar to using CONFIG_PM_DFS_INIT_AUTO, but with light_sleep_enable=true
    esp_pm_config_esp32s3_t cfg = {
        .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
        .min_freq_mhz = esp_clk_xtal_freq() / 1000000,
		.light_sleep_enable = true
    };

    ESP_ERROR_CHECK (esp_pm_configure(&cfg));
#endif
	uint8_t* mem = heap_caps_malloc (bufSize, MALLOC_CAP_SPIRAM);
	assert (mem);

	uint32_t chksum = fill (mem, bufSize);
	check (mem, bufSize, chksum, "init");

	for (size_t waitTime = 600; ; ++waitTime) {
		ESP_LOGI (TAG, "Sleeping for %zu s", waitTime);

		vTaskDelay (pdMS_TO_TICKS(waitTime * 1000));

		ESP_LOGI (TAG, "Checking");

		if (!check (mem, bufSize, chksum, "loop")) {
			// Sleep forever
			while (1) {
				vTaskDelay(pdMS_TO_TICKS(10000));
			}
		}
	}
}
