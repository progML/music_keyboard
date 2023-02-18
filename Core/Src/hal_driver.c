#include "gpio.h"
#include "hal_driver.h"

void resetAllDioids() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

void setLightGreenDiode() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}

void setOffGreenDiode() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

void setLightRedDiode() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
}

void setOffRedDiode() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
}

void setLightYellowDiode() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void setOffYellowDiode() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
}

void delay(int ms) {
    HAL_Delay(ms);
}

void playAnimation() {
    for (int i = 0; i < 2; i++) {
        setLightGreenDiode();
        setLightYellowDiode();
        delay(500);
        setOffGreenDiode();
        setOffYellowDiode();
        delay(500);
    }
}

int getButtonState() {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
}

long getCurrentTime(){
	return HAL_GetTick();
}

void blinkGreenDiode() {
    setOffGreenDiode();
    delay(500);
    setLightGreenDiode();
    delay(500);
    setOffGreenDiode();
    delay(500);
}
