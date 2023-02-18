#ifndef LAB1_HAL_DRIVER_H
#define LAB1_HAL_DRIVER_H

void resetAllDioids();

void setLightGreenDiode();

void setOffGreenDiode();

void setLightRedDiode();

void setOffRedDiode();

void setLightYellowDiode();

void setOffYellowDiode();

void delay(int ms);

void playAnimation();

int getButtonState();

void blinkGreenDiode();

long getCurrentTime();

#endif //LAB1_HAL_DRIVER_H