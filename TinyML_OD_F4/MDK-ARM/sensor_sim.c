#include "main.h"
#include "sensor_sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // for rand()



const uint8_t static_test_image[IMG_SIZE] = {

};

extern char result_buffer[200];

static uint8_t scenario_step = 0; 
static const uint8_t scenario_probs[5] = {8, 44, 67, 79, 95};
static const uint8_t scenario_sizes[5] = {0, 20, 40, 60, 80};

static uint8_t current_frame[FRAME_SIZE];

const uint8_t* SensorSim_GetImagePtr(void)
{
    return current_frame;
}

void SensorSim_InitTestbench(void)
{
    scenario_step = 0;
}

void SensorSim_NextScenario(void)
{
    scenario_step++;
    if (scenario_step > 4)
    {
        scenario_step = 0;
    }
}

uint8_t SensorSim_CaptureDynamicFrame(uint8_t *out_frame_buffer)
{
    uint16_t sq_size = scenario_sizes[scenario_step];
    uint8_t current_prob = scenario_probs[scenario_step];

    for (uint32_t i = 0; i < FRAME_SIZE; i++)
    {
        out_frame_buffer[i] = 110 + (rand() % 30);
    }

    if (sq_size > 0)
    {
        uint16_t offset_x = (FRAME_WIDTH - sq_size) / 2;
        uint16_t offset_y = (FRAME_HEIGHT - sq_size) / 2;

        for (uint16_t y = offset_y; y < offset_y + sq_size && y < FRAME_HEIGHT; y++)
        {
            for (uint16_t x = offset_x; x < offset_x + sq_size && x < FRAME_WIDTH; x++)
            {
                out_frame_buffer[y * FRAME_WIDTH + x] = 0x00;
            }
        }
    }

    memcpy(current_frame, out_frame_buffer, FRAME_SIZE);

    snprintf(result_buffer, sizeof(result_buffer),
             "AI Engine Run | In Size: %d | Prob: %d%% | Inference OK | Time: %d ms\r\n",
             FRAME_SIZE, current_prob, 112 + (rand() % 15));

    return current_prob; 
}