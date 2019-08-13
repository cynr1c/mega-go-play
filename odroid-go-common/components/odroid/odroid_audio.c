#include "odroid_audio.h"


#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "driver/i2s.h"
#include "driver/rtc_io.h"



#define I2S_NUM (I2S_NUM_0)

//1 is full levels, 2 half, 3 quarter,... note that prescaling sacrifices audio resolution!
#define SAMPLE_PRESCALER	(1)

static int AudioSink = ODROID_AUDIO_SINK_SPEAKER;
static odroid_volume_level volumeLevel = ODROID_VOLUME_LEVEL3;
static odroid_volume_level preMuteVolumeLevel = 0;
static int audio_sample_rate;


odroid_volume_level odroid_audio_volume_get()
{
    return volumeLevel;
}

void odroid_audio_volume_set(odroid_volume_level value)
{
    if (value >= ODROID_VOLUME_LEVEL_COUNT)
    {
        printf("odroid_audio_volume_set: value out of range (%d)\n", value);
        abort();
    }

    volumeLevel = value;
}

void odroid_audio_volume_change()
{
    unsigned int level = (volumeLevel + 1) % ODROID_VOLUME_LEVEL_COUNT;
    odroid_audio_volume_set(level);

    odroid_settings_Volume_set(level);
}

void odroid_audio_volume_increase()
{
    unsigned int level = (volumeLevel == ODROID_VOLUME_LEVEL0) ? preMuteVolumeLevel + 1 : volumeLevel + 1;

    if (level >= ODROID_VOLUME_LEVEL_COUNT)
    {
        odroid_audio_volume_set(ODROID_VOLUME_LEVEL_COUNT - 1);
        odroid_settings_Volume_set(ODROID_VOLUME_LEVEL_COUNT - 1);
        preMuteVolumeLevel = 0;
    }
    else
    {
        odroid_audio_volume_set(level);
        odroid_settings_Volume_set(level);
        preMuteVolumeLevel = 0;
    }
}

void odroid_audio_volume_decrease()
{
    unsigned int level = (volumeLevel == ODROID_VOLUME_LEVEL0) ? preMuteVolumeLevel - 1 : volumeLevel - 1;

    if (level < ODROID_VOLUME_LEVEL1)
    {
        odroid_audio_volume_set(ODROID_VOLUME_LEVEL1);
        odroid_settings_Volume_set(ODROID_VOLUME_LEVEL1);
        preMuteVolumeLevel = 0;
    }
    else
    {
        odroid_audio_volume_set(level);
        odroid_settings_Volume_set(level);
        preMuteVolumeLevel = 0;
    }
}

void odroid_audio_volume_mute()
{
    if (volumeLevel == ODROID_VOLUME_LEVEL0)
    {
        odroid_audio_volume_set(preMuteVolumeLevel);
        odroid_settings_Volume_set(preMuteVolumeLevel);
        preMuteVolumeLevel = 0;
    }
    else
    {
        preMuteVolumeLevel = volumeLevel;
        odroid_audio_volume_set(ODROID_VOLUME_LEVEL0);
        odroid_settings_Volume_set(ODROID_VOLUME_LEVEL0);
    }
}

void odroid_audio_init(ODROID_AUDIO_SINK sink, int sample_rate)
{
    printf("%s: sink=%d, sample_rate=%d\n", __func__, sink, sample_rate);

    AudioSink = sink;
    audio_sample_rate = sample_rate;

    // NOTE: buffer needs to be adjusted per AUDIO_SAMPLE_RATE
    if(AudioSink == ODROID_AUDIO_SINK_SPEAKER)
    {
        i2s_config_t i2s_config = {
            //.mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
            .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
            .sample_rate = audio_sample_rate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            //.communication_format = I2S_COMM_FORMAT_PCM,
            .dma_buf_count = 8,
            //.dma_buf_len = 1472 / 2,  // (368samples * 2ch * 2(short)) = 1472
            .dma_buf_len = 534,  // (416samples * 2ch * 2(short)) = 1664
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
            .use_apll = 0 //1
        };

        i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

        i2s_set_pin(I2S_NUM, NULL);
        i2s_set_dac_mode(/*I2S_DAC_CHANNEL_LEFT_EN*/ I2S_DAC_CHANNEL_BOTH_EN);
    }
    else if (AudioSink == ODROID_AUDIO_SINK_DAC)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
            .sample_rate = audio_sample_rate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
            .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
            .dma_buf_count = 8,
            //.dma_buf_len = 1472 / 2,  // (368samples * 2ch * 2(short)) = 1472
            .dma_buf_len = 534,  // (416samples * 2ch * 2(short)) = 1664
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
            .use_apll = 1
        };

        i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

        i2s_pin_config_t pin_config = {
            .bck_io_num = 4,
            .ws_io_num = 12,
            .data_out_num = 15,
            .data_in_num = -1                                                       //Not used
        };
        i2s_set_pin(I2S_NUM, &pin_config);


        // Disable internal amp
        esp_err_t err;

        err = gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
        if (err != ESP_OK)
        {
            abort();
        }

        err = gpio_set_direction(GPIO_NUM_26, GPIO_MODE_DISABLE);
        if (err != ESP_OK)
        {
            abort();
        }

        err = gpio_set_level(GPIO_NUM_25, 0);
        if (err != ESP_OK)
        {
            abort();
        }
    }
    else
    {
        abort();
    }

    odroid_volume_level level = odroid_settings_Volume_get();
    odroid_audio_volume_set(level);
}

void odroid_audio_terminate()
{
    i2s_zero_dma_buffer(I2S_NUM);
    i2s_stop(I2S_NUM);

    i2s_start(I2S_NUM);


    esp_err_t err = rtc_gpio_init(GPIO_NUM_25);
    err = rtc_gpio_init(GPIO_NUM_26);
    if (err != ESP_OK)
    {
        abort();
    }

    err = rtc_gpio_set_direction(GPIO_NUM_25, RTC_GPIO_MODE_OUTPUT_ONLY);
    err = rtc_gpio_set_direction(GPIO_NUM_26, RTC_GPIO_MODE_OUTPUT_ONLY);
    if (err != ESP_OK)
    {
        abort();
    }

    err = rtc_gpio_set_level(GPIO_NUM_25, 0);
    err = rtc_gpio_set_level(GPIO_NUM_26, 0);
    if (err != ESP_OK)
    {
        abort();
    }
}

static inline void __attribute__((optimize("O3"))) odroid_audio_scale_sample(uint16_t sample16, int vol, uint16_t * dac0, uint16_t* dac1)
{
	//we've got 2 seperate dacs, use them!
	switch (vol)
	{
		case ODROID_VOLUME_LEVEL0:
			*dac0=0;
			*dac1=0;
			break;
		case ODROID_VOLUME_LEVEL1:
			//1-7/8=1/8
			*dac0=sample16;
			*dac1=((uint32_t)sample16*7 + 4)>>3;
			break;
		case ODROID_VOLUME_LEVEL2:
			//1-3/4=1/4
			*dac0=sample16;
			*dac1=((uint32_t)sample16*3 + 2)>>2;
			break;
		case ODROID_VOLUME_LEVEL3:
			//1, no error
			*dac0=sample16;
			*dac1=0x8000;
			break;
		case ODROID_VOLUME_LEVEL4:
		default:
			//2, no error
			*dac0=sample16;
			*dac1=~sample16;
			break;
	}
}

void odroid_audio_submit(short* stereoAudioBuffer, int frameCount)
{
    short currentAudioSampleCount = frameCount * 2;

    if (AudioSink == ODROID_AUDIO_SINK_SPEAKER)
    {
        // Convert for built in DAC
        for (short i = 0; i < currentAudioSampleCount; i += 2)
        {
            uint16_t dac0;
            uint16_t dac1;

			//replace the horrible float-based and partially just plain wrong sample processing
            //downmix - samples are 16bit, dac only uses 8 msbs
            int32_t sample=stereoAudioBuffer[i] + stereoAudioBuffer[i+1];	//sign + max 16bit
#ifdef SAMPLE_PRESCALER
            //use simple integer shiftable divisions - compiler should take care of sign bit
			sample >>= SAMPLE_PRESCALER;
#endif
			uint16_t sample16=(sample+0x8000)&0xffff;
            odroid_audio_scale_sample(sample16, volumeLevel, &dac0, &dac1);

            stereoAudioBuffer[i] = (short) dac1;
            stereoAudioBuffer[i + 1] = (short) dac0;
        }

        int len = currentAudioSampleCount * 2;
        int count = i2s_write_bytes(I2S_NUM, stereoAudioBuffer, len, portMAX_DELAY);
        if (count != len)
        {
            printf("i2s_write_bytes: count (%d) != len (%d)\n", count, len);
            abort();
        }
    }
    else if (AudioSink == ODROID_AUDIO_SINK_DAC)
    {
        int len = currentAudioSampleCount * sizeof(int16_t);

        for (short i = 0; i < currentAudioSampleCount; ++i)
        {
            int sample;	//use simple integer shiftable divisions
            switch (volumeLevel)
            {
				case ODROID_VOLUME_LEVEL0:
					sample = 0;
					break;
				case ODROID_VOLUME_LEVEL1:
					sample = stereoAudioBuffer[i] / 8;
					break;
				case ODROID_VOLUME_LEVEL2:
					sample = stereoAudioBuffer[i] / 4;
					break;
				case ODROID_VOLUME_LEVEL3:
					sample = stereoAudioBuffer[i] / 2;
					break;
				case ODROID_VOLUME_LEVEL4:
				default:
					sample = stereoAudioBuffer[i];
					break;
            }

            if (sample > 32767)
                sample = 32767;
            else if (sample < -32768)
                sample = -32768;

            stereoAudioBuffer[i] = (short)sample;
        }

        int count = i2s_write_bytes(I2S_NUM, (const char *)stereoAudioBuffer, len, portMAX_DELAY);
        if (count != len)
        {
            printf("i2s_write_bytes: count (%d) != len (%d)\n", count, len);
            abort();
        }
    }
    else
    {
        abort();
    }
}

int odroid_audio_sample_rate_get()
{
    return audio_sample_rate;
}
