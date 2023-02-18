//
// Created by ad3st on 30.11.2021.
//

#include "sound_driver.h"
#include "tim.h"
#include "hal_driver.h"

void sound_driver_init(void) {
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Init(&htim2);
};


void sound_driver_set_frequency(uint16_t freq) {
    TIM2->PSC = ((2 * HAL_RCC_GetPCLK1Freq()) / (2 * 10 * freq)) - 1;
};

void sound_driver_volume_on() {
    TIM2->CCR1 = 10;
};

void sound_driver_volume_mute() {
    TIM2->CCR1 = 0;
};


int melody_tick = 0;
int new = 1;
int played = 0;

void sound_driver_tone(int melody, int duration) {
    if (new) {
        new = 0;
        melody_tick = HAL_GetTick();
    }
    if (!new && HAL_GetTick() - melody_tick < duration){
        if (HAL_GetTick() - melody_tick < duration* 0.9){
            sound_driver_set_frequency(melody);
            sound_driver_volume_on();
            played = 1;
            new = 1;
        }
        else
            sound_driver_volume_mute();
    }
    else {
        new = 1;
        played = 0;
        sound_driver_volume_mute();
    }
//    HAL_Delay(duration * 0.9);
//    sound_driver_volume_mute();
//    HAL_Delay(duration * 0.1);
}


//void sound_driver_play(int *melody, uint16_t len) {
void sound_driver_play(int cur_melody,int cur_pause, int tempo) {
        int divider = 0, noteDuration = 0;
        // this calculates the duration of a whole note in ms
//        for (int i = 0; i < len; i = i + 2) {
//            int cur_melody = melody[i];
//            int cur_pause = melody[i + 1];
//        if (cur_melody != 0) {
//            sound_driver_set_frequency(cur_melody);
//            sound_driver_volume_on();
//            noteDuration = (wholenote) / divider;
//            HAL_Delay(TEMPO / cur_pause);
////            HAL_Delay(noteDuration * 0.9);
//            sound_driver_volume_mute();
//        } else
//            HAL_Delay(TEMPO / cur_pause);
//        HAL_Delay(10);
            if (cur_pause > 0)
                noteDuration = wholeNote(tempo) / cur_pause;
            else if (cur_pause < 0) {
                noteDuration = wholeNote(tempo) / abs(cur_pause);
                noteDuration *= 1.5;
            }
            sound_driver_tone(cur_melody, noteDuration);
//        }
    };