/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "hal_driver.h"
#include <stdio.h>
#include "kb.h"
#include "sound_driver.h"
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
#define BUFFER_SIZE 32


uint8_t buffer[BUFFER_SIZE];

size_t write_pointer = 0;
size_t read_pointer = 0;

enum MathOperation {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    EQUAL,
    NONE
};

int mode = 0;
char ch[] = "F";
char message[150] = "";
char mode0[] = "Mode: Preriv \n\r";
char mode1[] = "Mode: Bez Preriv \n\r";
int canReceive = 1;
short isFirst = 1;
short isJustReset = 1;
short counter = 0;
short firstOperand = 0;
short secondOperand = 0;
enum MathOperation operation = NONE;

char calcChars[] = "0123456789";
char calcCharsOperations[] = "+-*/=";
short curCalcsOperation = 0;
char errorMessage[] = "ERROR";
char errorMessageDiv[] = "Cant divide by zero";

//LAB4

#define FONT Font_7x10 // MB CHANGE

void writeScreenMessage(char *message) {
    oled_WriteString(message, FONT, White);
    oled_UpdateScreen();
}

void writeScreenChar(char ch) {
    oled_WriteChar(ch, FONT, White);
    oled_UpdateScreen();
}

void resetScreen() {
    oled_Fill(Black);
    oled_SetCursor(0, 0);
    oled_UpdateScreen();
}

void writeScreenError() {
    writeScreenMessage(errorMessage);
    oled_UpdateScreen();
}

void writeScreenErrorAndReset() {
    writeScreenError();
    resetCalc();
    oled_UpdateScreen();
}




void nextOperation(){
    curCalcsOperation++;
    if (curCalcsOperation > 3)
        curCalcsOperation = 0;
}

short checkNumbertoOverflow(short fullNumber, short number) {
    if ((fullNumber == 3276 && number >= 7) || fullNumber > 3276)
        return -1;
    return fullNumber * 10 + number;
}

short getNumber(char ch) {
    short asciNum = ch;
    if (asciNum >= 48 && asciNum <= 57)
        return asciNum - '0';
    return -1;
}

enum MathOperation getMathOperation(char ch) {
    switch (ch) {
        case '+':
            return ADDITION;
        case '-':
            return SUBTRACTION;
        case '*':
            return MULTIPLICATION;
        case '/':
            return DIVISION;
        case '=':
            return EQUAL;
    }
    return NONE;
}

void handle_calc(char ch) {
    if (isJustReset) {
        resetScreen();
        isJustReset = 0;
    }
    snprintf(message, sizeof(message),
                             "Key in calc pressed is %d\r\n", ch);
                    sendMessage(message);
    enum MathOperation operationBuf = getMathOperation(ch);
    if (operationBuf == NONE){
    	if (!isFirst && counter == 0)
    		oled_SetCursor(0, 20);
    	writeScreenChar(ch);
    }
    if (operationBuf != NONE && operationBuf != EQUAL && (!isFirst && counter == 0 ||  isFirst && counter > 0)) {
                oled_SetCursor(0, 10);
                writeScreenChar(ch);
                operation = operationBuf;
                isFirst = 0;
                counter = 0;
                oled_UpdateScreen();
                return;
            }

    if (counter > 0) {
        if (isFirst && operationBuf != NONE && operationBuf != EQUAL) {
            oled_SetCursor(0, 10);
            writeScreenChar(ch);
            operation = operationBuf;
            isFirst = 0;
            counter = 0;
            oled_UpdateScreen();
            return;
        }
        if (!isFirst && operationBuf != NONE && operationBuf != EQUAL){
        	writeScreenErrorAndReset();
        	return;
        }
        if (!isFirst && operationBuf != NONE && operationBuf == EQUAL) {
        	oled_SetCursor(0, 30);
        	writeScreenChar(ch);
        	oled_SetCursor(0, 40);
            long testOverflow;
            short res;
            switch (operation) {
                case ADDITION: {
                    testOverflow = firstOperand + secondOperand;
                    res = firstOperand + secondOperand;
                    break;
                }
                case SUBTRACTION: {
                    testOverflow = firstOperand - secondOperand;
                    res = firstOperand - secondOperand;
                    break;
                }
                case MULTIPLICATION: {
                    testOverflow = firstOperand * secondOperand;
                    res = firstOperand * secondOperand;
                    break;
                }
                case DIVISION: {
                    if (secondOperand == 0) {
                        //sprintf(message, "\n\rCant divide by zero\n\r");
                        writeScreenMessage(errorMessageDiv);
                        resetCalc();
                        return;
                    }
                    testOverflow = firstOperand / secondOperand;
                    res = firstOperand / secondOperand;
                    break;
                }
            }
            if (res == testOverflow) {
                sprintf(message, "%d", res);
                writeScreenMessage(message);
            } else {
                writeScreenError();
            }
            resetCalc();
            oled_UpdateScreen();
            return;
        }
    }
    if (operationBuf != NONE || counter > 4) {
        writeScreenErrorAndReset();
        return;
    }


    short number = getNumber(ch);
    if (number != -1) {
        if (isFirst) {
            short numberBuf = checkNumbertoOverflow(firstOperand, number);
            if (numberBuf == -1) {
                writeScreenErrorAndReset();
                return;
            }
            firstOperand = numberBuf;
        } else {
            short numberBuf = checkNumbertoOverflow(secondOperand, number);
            if (numberBuf == -1) {
                writeScreenErrorAndReset();
                return;
            }
            secondOperand = numberBuf;
        }
        counter++;
    } else {
        writeScreenErrorAndReset();
    }
}




short makeMathOperation(enum MathOperation operation, short firstOperand, short secondOperand) {
    switch (operation) {
        case ADDITION: {
            long testOverflow = firstOperand + secondOperand;
            short result = firstOperand + secondOperand;
            if (result == testOverflow)
                return result;
            else
                return -1;
        }
        case SUBTRACTION:
            return firstOperand - secondOperand;
        case MULTIPLICATION: {
            long testOverflow = firstOperand * secondOperand;
            short result = firstOperand * secondOperand;
            if (result == testOverflow)
                return result;
            else
                return -1;
        }
        case DIVISION: {
            return firstOperand / secondOperand;
        }
    }
    return -1;
}

void
resetCalc() {
    isFirst = 1;
    counter = 0;
    firstOperand = 0;
    secondOperand = 0;
    operation = NONE;
    isJustReset = 1;
}

void buffer_init() {
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        buffer[i] = 0;
}

void buffer_add(uint8_t num) {
    buffer[write_pointer] = num;
    write_pointer = (write_pointer + 1) % BUFFER_SIZE;
}

int buffer_read() {
    if (read_pointer == write_pointer) {
        return -1;
    }
    uint8_t num = buffer[read_pointer];
    read_pointer = (read_pointer + 1) % BUFFER_SIZE;
    return num;
}

void keyboard_read(void) {
//    static uint8_t const rows[4] = { 0xF7, 0x7B, 0x3D, 0x1E };
    static uint8_t const rows[4] = {0x1E, 0x3D, 0x7B, 0xF7};
    static int current_row = 0;
    static int row_result[4] = {0, 0, 0, 0};

    if (ks_state == 0) {
        if (row_result[current_row] != ks_result) {
            uint8_t keyNum = 0;
            if (ks_result & 1) {
                buffer_add(3 * current_row + 3);
            }
            if (ks_result & 2) {
                buffer_add(3 * current_row + 2);
            }
            if (ks_result & 4) {
                buffer_add(3 * current_row + 1);
            }
        }

        row_result[current_row] = ks_result;
        current_row = (current_row + 1) % 4;
        ks_current_row = rows[current_row];
        ks_continue();
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == &hi2c1 && ks_state) {
        ks_continue();
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == &hi2c1 && ks_state) {
        ks_continue();
    }
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim->Instance == TIM6) {
//        keyboard_read();
//    }
//}

typedef enum {
    INPUT_PORT = 0x00, //Read byte XXXX XXXX
    OUTPUT_PORT = 0x01, //Read/write byte 1111 1111
    POLARITY_INVERSION = 0x02, //Read/write byte 0000 0000
    CONFIG = 0x03 //Read/write byte 1111 1111
} pca9538_regs_t;



void sendMessage(char *message) {
    int size = strlen(message);
//    if (mode)
    HAL_UART_Transmit(&huart6, (uint8_t *) message, size, 100);
//    else {
////    	delay(1000);
//        long time = getCurrentTime();
//        HAL_StatusTypeDef isReaded1 = HAL_BUSY;
//        int curtime = getCurrentTime();
//        int counter = 0;
//        while (counter < 100000 && isReaded1 == HAL_BUSY) {
//            isReaded1 = HAL_UART_Transmit_IT(&huart6, (uint8_t *) message, size);
//            curtime = getCurrentTime();
//            counter++;
//        }
//    }
}

void sendError() {
    sendMessage(errorMessage);
}

void sendErrorAndReset() {
    sendError();
    resetCalc();
}

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {

int TEMPO_AHA = 140;

int melody_AHA[] = {

        // Take on me, by A-ha
        // Score available at https://musescore.com/user/27103612/scores/4834399
        // Arranged by Edward Truong

        NOTE_FS5, 8, NOTE_FS5, 8, NOTE_D5, 8, NOTE_B4, 8, REST, 8, NOTE_B4, 8, REST, 8, NOTE_E5, 8,
        REST, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, NOTE_GS5, 8, NOTE_GS5, 8, NOTE_A5, 8, NOTE_B5, 8,
        NOTE_A5, 8, NOTE_A5, 8, NOTE_A5, 8, NOTE_E5, 8, REST, 8, NOTE_D5, 8, REST, 8, NOTE_FS5, 8,
        REST, 8, NOTE_FS5, 8, REST, 8, NOTE_FS5, 8, NOTE_E5, 8, NOTE_E5, 8, NOTE_FS5, 8, NOTE_E5, 8,
        NOTE_FS5, 8, NOTE_FS5, 8, NOTE_D5, 8, NOTE_B4, 8, REST, 8, NOTE_B4, 8, REST, 8, NOTE_E5, 8,

        REST, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, NOTE_GS5, 8, NOTE_GS5, 8, NOTE_A5, 8, NOTE_B5, 8,
        NOTE_A5, 8, NOTE_A5, 8, NOTE_A5, 8, NOTE_E5, 8, REST, 8, NOTE_D5, 8, REST, 8, NOTE_FS5, 8,
        REST, 8, NOTE_FS5, 8, REST, 8, NOTE_FS5, 8, NOTE_E5, 8, NOTE_E5, 8, NOTE_FS5, 8, NOTE_E5, 8,
        NOTE_FS5, 8, NOTE_FS5, 8, NOTE_D5, 8, NOTE_B4, 8, REST, 8, NOTE_B4, 8, REST, 8, NOTE_E5, 8,
        REST, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, NOTE_GS5, 8, NOTE_GS5, 8, NOTE_A5, 8, NOTE_B5, 8,

        NOTE_A5, 8, NOTE_A5, 8, NOTE_A5, 8, NOTE_E5, 8, REST, 8, NOTE_D5, 8, REST, 8, NOTE_FS5, 8,
        REST, 8, NOTE_FS5, 8, REST, 8, NOTE_FS5, 8, NOTE_E5, 8, NOTE_E5, 8, NOTE_FS5, 8, NOTE_E5, 8,

};

int TEMPO_STARWARS = 108;

int melody_STARWARS[] = {

        // Dart Vader theme (Imperial March) - Star wars
        // Score available at https://musescore.com/user/202909/scores/1141521
        // The tenor saxophone part was used

        NOTE_AS4, 8, NOTE_AS4, 8, NOTE_AS4, 8,//1
        NOTE_F5, 2, NOTE_C6, 2,
        NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F6, 2, NOTE_C6, 4,
        NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F6, 2, NOTE_C6, 4,
        NOTE_AS5, 8, NOTE_A5, 8, NOTE_AS5, 8, NOTE_G5, 2, NOTE_C5, 8, NOTE_C5, 8, NOTE_C5, 8,
        NOTE_F5, 2, NOTE_C6, 2,
        NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F6, 2, NOTE_C6, 4,

        NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F6, 2, NOTE_C6, 4, //8
        NOTE_AS5, 8, NOTE_A5, 8, NOTE_AS5, 8, NOTE_G5, 2, NOTE_C5, -8, NOTE_C5, 16,
        NOTE_D5, -4, NOTE_D5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,
        NOTE_F5, 8, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 4, NOTE_D5, 8, NOTE_E5, 4, NOTE_C5, -8, NOTE_C5, 16,
        NOTE_D5, -4, NOTE_D5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,

        NOTE_C6, -8, NOTE_G5, 16, NOTE_G5, 2, REST, 8, NOTE_C5, 8,//13
        NOTE_D5, -4, NOTE_D5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,
        NOTE_F5, 8, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 4, NOTE_D5, 8, NOTE_E5, 4, NOTE_C6, -8, NOTE_C6, 16,
        NOTE_F6, 4, NOTE_DS6, 8, NOTE_CS6, 4, NOTE_C6, 8, NOTE_AS5, 4, NOTE_GS5, 8, NOTE_G5, 4, NOTE_F5, 8,
        NOTE_C6, 1

};

int TEMPO_TETRIS = 144;

int melody_tetris[] = {

        //Based on the arrangement at https://www.flutetunes.com/tunes.php?id=192

        NOTE_E5, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_C5, 8, NOTE_B4, 8,
        NOTE_A4, 4, NOTE_A4, 8, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
        NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
        NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 8, NOTE_A4, 4, NOTE_B4, 8, NOTE_C5, 8,

        NOTE_D5, -4, NOTE_F5, 8, NOTE_A5, 4, NOTE_G5, 8, NOTE_F5, 8,
        NOTE_E5, -4, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
        NOTE_B4, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
        NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 4, REST, 4,

        NOTE_E5, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_C5, 8, NOTE_B4, 8,
        NOTE_A4, 4, NOTE_A4, 8, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
        NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
        NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 8, NOTE_A4, 4, NOTE_B4, 8, NOTE_C5, 8,

        NOTE_D5, -4, NOTE_F5, 8, NOTE_A5, 4, NOTE_G5, 8, NOTE_F5, 8,
        NOTE_E5, -4, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
        NOTE_B4, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
        NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 4, REST, 4,


        NOTE_E5, 2, NOTE_C5, 2,
        NOTE_D5, 2, NOTE_B4, 2,
        NOTE_C5, 2, NOTE_A4, 2,
        NOTE_GS4, 2, NOTE_B4, 4, REST, 8,
        NOTE_E5, 2, NOTE_C5, 2,
        NOTE_D5, 2, NOTE_B4, 2,
        NOTE_C5, 4, NOTE_E5, 4, NOTE_A5, 2,
        NOTE_GS5, 2,

};

int melody_Potter[] = {


        // Hedwig's theme fromn the Harry Potter Movies
        // Socre from https://musescore.com/user/3811306/scores/4906610

        REST, 2, NOTE_D4, 4,
        NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4,
        NOTE_G4, 2, NOTE_D5, 4,
        NOTE_C5, -2,
        NOTE_A4, -2,
        NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4,
        NOTE_F4, 2, NOTE_GS4, 4,
        NOTE_D4, -1,
        NOTE_D4, 4,

        NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4, //10
        NOTE_G4, 2, NOTE_D5, 4,
        NOTE_F5, 2, NOTE_E5, 4,
        NOTE_DS5, 2, NOTE_B4, 4,
        NOTE_DS5, -4, NOTE_D5, 8, NOTE_CS5, 4,
        NOTE_CS4, 2, NOTE_B4, 4,
        NOTE_G4, -1,
        NOTE_AS4, 4,

        NOTE_D5, 2, NOTE_AS4, 4,//18
        NOTE_D5, 2, NOTE_AS4, 4,
        NOTE_DS5, 2, NOTE_D5, 4,
        NOTE_CS5, 2, NOTE_A4, 4,
        NOTE_AS4, -4, NOTE_D5, 8, NOTE_CS5, 4,
        NOTE_CS4, 2, NOTE_D4, 4,
        NOTE_D5, -1,
        REST, 4, NOTE_AS4, 4,

        NOTE_D5, 2, NOTE_AS4, 4,//26
        NOTE_D5, 2, NOTE_AS4, 4,
        NOTE_F5, 2, NOTE_E5, 4,
        NOTE_DS5, 2, NOTE_B4, 4,
        NOTE_DS5, -4, NOTE_D5, 8, NOTE_CS5, 4,
        NOTE_CS4, 2, NOTE_AS4, 4,
        NOTE_G4, -1,

};

int TEMPO_POTTER = 122;

int TEMPO_MII = 114;

int melody_MII[] = {

        // Mii Channel theme
        // Score available at https://musescore.com/user/16403456/scores/4984153
        // Uploaded by Catalina Andrade

        NOTE_FS4, 8, REST, 8, NOTE_A4, 8, NOTE_CS5, 8, REST, 8, NOTE_A4, 8, REST, 8, NOTE_FS4, 8, //1
        NOTE_D4, 8, NOTE_D4, 8, NOTE_D4, 8, REST, 8, REST, 4, REST, 8, NOTE_CS4, 8,
        NOTE_D4, 8, NOTE_FS4, 8, NOTE_A4, 8, NOTE_CS5, 8, REST, 8, NOTE_A4, 8, REST, 8, NOTE_F4, 8,
        NOTE_E5, -4, NOTE_DS5, 8, NOTE_D5, 8, REST, 8, REST, 4,

        NOTE_GS4, 8, REST, 8, NOTE_CS5, 8, NOTE_FS4, 8, REST, 8, NOTE_CS5, 8, REST, 8, NOTE_GS4, 8, //5
        REST, 8, NOTE_CS5, 8, NOTE_G4, 8, NOTE_FS4, 8, REST, 8, NOTE_E4, 8, REST, 8,
        NOTE_E4, 8, NOTE_E4, 8, NOTE_E4, 8, REST, 8, REST, 4, NOTE_E4, 8, NOTE_E4, 8,
        NOTE_E4, 8, REST, 8, REST, 4, NOTE_DS4, 8, NOTE_D4, 8,

        NOTE_CS4, 8, REST, 8, NOTE_A4, 8, NOTE_CS5, 8, REST, 8, NOTE_A4, 8, REST, 8, NOTE_FS4, 8, //9
        NOTE_D4, 8, NOTE_D4, 8, NOTE_D4, 8, REST, 8, NOTE_E5, 8, NOTE_E5, 8, NOTE_E5, 8, REST, 8,
        REST, 8, NOTE_FS4, 8, NOTE_A4, 8, NOTE_CS5, 8, REST, 8, NOTE_A4, 8, REST, 8, NOTE_F4, 8,
        NOTE_E5, 2, NOTE_D5, 8, REST, 8, REST, 4,

        NOTE_B4, 8, NOTE_G4, 8, NOTE_D4, 8, NOTE_CS4, 4, NOTE_B4, 8, NOTE_G4, 8, NOTE_CS4, 8, //13
        NOTE_A4, 8, NOTE_FS4, 8, NOTE_C4, 8, NOTE_B3, 4, NOTE_F4, 8, NOTE_D4, 8, NOTE_B3, 8,
        NOTE_E4, 8, NOTE_E4, 8, NOTE_E4, 8, REST, 4, REST, 4, NOTE_AS4, 4,
        NOTE_CS5, 8, NOTE_D5, 8, NOTE_FS5, 8, NOTE_A5, 8, REST, 8, REST, 4,

        REST, 2, NOTE_A3, 4, NOTE_AS3, 4, //17
        NOTE_A3, -4, NOTE_A3, 8, NOTE_A3, 2,
        REST, 4, NOTE_A3, 8, NOTE_AS3, 8, NOTE_A3, 8, NOTE_F4, 4, NOTE_C4, 8,
        NOTE_A3, -4, NOTE_A3, 8, NOTE_A3, 2,

        REST, 2, NOTE_B3, 4, NOTE_C4, 4, //21
        NOTE_CS4, -4, NOTE_C4, 8, NOTE_CS4, 2,
        REST, 4, NOTE_CS4, 8, NOTE_C4, 8, NOTE_CS4, 8, NOTE_GS4, 4, NOTE_DS4, 8,
        NOTE_CS4, -4, NOTE_DS4, 8, NOTE_B3, 1,

        NOTE_E4, 4, NOTE_E4, 4, NOTE_E4, 4, REST, 8,//25

};

int TEMPO = 144;

//int wholenote = (60000 * 4) / TEMPO;

int wholeNote(int tempo) {
    return (60000 * 4) / tempo;
}

enum MusicBoxStatus {
    MUSIC_BOX_PLAYING,
    MUSIC_BOX_EDITING,
    MUSIC_BOX_EDITING_TEMPO,
    MUSIC_BOX_EDITING_NOTE,
    MUSIC_BOX_EDITING_DELAY
};

enum MusicBoxStatus currentMusicBoxStatus = MUSIC_BOX_PLAYING;

#define CUSTOM_MELODY_LENGTH 64


int custom_melody[CUSTOM_MELODY_LENGTH];

int custom_melody_size = 0;

int custom_tempo = 0;

void custom_melody_init() {
    for (size_t i = 0; i < CUSTOM_MELODY_LENGTH; i = i + 2) {
        custom_melody[i] = 0;
        custom_melody[i + 1] = 8;
    }
}

int currentTempo = 0;
int currentMelodySize = 0;

int *currentMelody;

int currentMelodyPosition = 0;

void moveToNextMelodyPosition() {
    currentMelodyPosition++;
    if (currentMelodyPosition > currentMelodySize)
        currentMelodyPosition = 0;
}

void handleInputEditingNote(int pressed_key) {
    switch (pressed_key) {
        case 1:
            custom_melody[custom_melody_size] = NOTE_A4;
            break;
        case 2:
            custom_melody[custom_melody_size] = NOTE_D4;
            break;
        case 3:
            custom_melody[custom_melody_size] = NOTE_G4;
            break;
        case 4:
            custom_melody[custom_melody_size] = NOTE_E5;
            break;
        case 5:
            custom_melody[custom_melody_size] = NOTE_CS5;
            break;
        case 6:
            custom_melody[custom_melody_size] = NOTE_D7;
            break;
        case 7:
            custom_melody[custom_melody_size] = NOTE_G7;
            break;
        case 8:
            custom_melody[custom_melody_size] = NOTE_C8;
            break;
        case 9:
            snprintf(message, sizeof(message),
                     "Current melody to add %d\r\n", custom_melody[custom_melody_size]);
            sendMessage(message);
            break;
        case 12:
            snprintf(message, sizeof(message),
                     "Added melody %d\r\n", custom_melody[custom_melody_size]);
            sendMessage(message);
            custom_melody_size++;
            currentMusicBoxStatus = MUSIC_BOX_EDITING_DELAY;
            break;
    }
}

void handleInputEditingDelay(int pressed_key) {
    switch (pressed_key) {
        case 1:
            custom_melody[custom_melody_size] = -16;
            break;
        case 2:
            custom_melody[custom_melody_size] = -8;
            break;
        case 3:
            custom_melody[custom_melody_size] = -4;
            break;
        case 4:
            custom_melody[custom_melody_size] = -2;
            break;
        case 5:
            custom_melody[custom_melody_size] = 2;
            break;
        case 6:
            custom_melody[custom_melody_size] = 4;
            break;
        case 7:
            custom_melody[custom_melody_size] = 8;
            break;
        case 8:
            custom_melody[custom_melody_size] = 16;
            break;
        case 9:
            snprintf(message, sizeof(message),
                     "Current delay to add: %d\r\n", custom_melody[custom_melody_size]);
            sendMessage(message);
            break;
        case 12:
            snprintf(message, sizeof(message),
                     "Added delay: %d\r\n", custom_melody[custom_melody_size]);
            sendMessage(message);
            custom_melody_size++;
            currentMusicBoxStatus = MUSIC_BOX_EDITING;
            break;
    }
}

void handleInputEditingTempo(int pressed_key) {
    switch (pressed_key) {
        case 1:
            custom_tempo = 100;
            break;
        case 2:
            custom_tempo = 110;
            break;
        case 3:
            custom_tempo = 115;
            break;
        case 4:
            custom_tempo = 120;
            break;
        case 5:
            custom_tempo = 125;
            break;
        case 6:
            custom_tempo = 130;
            break;
        case 7:
            custom_tempo = 135;
            break;
        case 8:
            custom_tempo = 140;
            break;
        case 9:
            snprintf(message, sizeof(message),
                     "The Custom tempo: %d\r\n", custom_tempo);
            sendMessage(message);
            break;
        case 12:
            currentMusicBoxStatus = MUSIC_BOX_EDITING;
            snprintf(message, sizeof(message),
                     "Applied tempo: %d\r\n", custom_tempo);
            sendMessage(message);
            break;
    }
}

void handleInputPlaying(int pressed_key) {
    switch (pressed_key) {
        case 1:
            currentMelody = melody_AHA;
            currentMelodySize = sizeof(melody_AHA) / sizeof(melody_AHA[0]);
            currentTempo = TEMPO_AHA;
            currentMelodyPosition = 0;
            snprintf(message, sizeof(message),
                     "Playing AHHA - Take on me\r\n");
            sendMessage(message);
            break;
        case 2:
            currentMelody = melody_Potter;
            currentMelodySize = sizeof(melody_Potter) / sizeof(melody_Potter[0]);
            currentTempo = TEMPO_POTTER;
            currentMelodyPosition = 0;
            snprintf(message, sizeof(message),
                     "Playing  Harry Potter - Hedwig's theme\r\n");
            sendMessage(message);
            break;
        case 3:
            currentMelody = melody_STARWARS;
            currentMelodySize = sizeof(melody_STARWARS) / sizeof(melody_STARWARS[0]);
            currentTempo = TEMPO_STARWARS;
            currentMelodyPosition = 0;
            snprintf(message, sizeof(message),
                     "Playing Star Wars - Imperial March\r\n");
            sendMessage(message);
            break;
        case 4:
            currentMelody = melody_MII;
            currentMelodySize = sizeof(melody_MII) / sizeof(melody_MII[0]);
            currentTempo = TEMPO_MII;
            currentMelodyPosition = 0;
            snprintf(message, sizeof(message),
                     "Playing  Mii channel theme\r\n");
            sendMessage(message);
            break;
        case 5:
            currentMelody = custom_melody;
            currentMelodySize = custom_melody_size;
            currentTempo = custom_tempo;
            currentMelodyPosition = 0;
            snprintf(message, sizeof(message),
                     "Playing  Custom Melody\r\n");
            sendMessage(message);
            break;
        case 6:
            currentMusicBoxStatus = MUSIC_BOX_EDITING;
            snprintf(message, sizeof(message),
                     "Editing Mode On\r\n");
            sendMessage(message);
            break;
    }
}

void handleInputEditing(int pressed_key) {
    switch (pressed_key) {
        case 7:
            snprintf(message, sizeof(message),
                     "Playing Music\r\n");
            sendMessage(message);
            currentMusicBoxStatus = MUSIC_BOX_PLAYING;
            break;
        case 8:
            snprintf(message, sizeof(message),
                     "Printing User Melody\r\n"
                     "Tempo: %d\r\n"
                     "Size: %d\r\n"
                     "Max_Size: %d\r\n"
                     "Notes: \r\n", custom_tempo, custom_melody_size, CUSTOM_MELODY_LENGTH);
            sendMessage(message);

            for (size_t i = 0; i < custom_melody_size; i = i + 2) {
                snprintf(message, sizeof(message), "%d %d | ",
                         custom_melody[i], custom_melody[i + 1]);
                sendMessage(message);
            }
            snprintf(message, sizeof(message), "\r\n");
            sendMessage(message);
            break;
        case 9:
            if (custom_melody_size + 2 <= CUSTOM_MELODY_LENGTH) {
                snprintf(message, sizeof(message),
                         "Adding Note\r\n");
                currentMusicBoxStatus = MUSIC_BOX_EDITING_NOTE;
            } else {
                snprintf(message, sizeof(message),
                         "Cannot add note, cut one by pressing 11\r\n");
            }
            sendMessage(message);
            break;
        case 10:
            snprintf(message, sizeof(message),
                     "Editing TEMPO\r\n");
            currentMusicBoxStatus = MUSIC_BOX_EDITING_TEMPO;
            sendMessage(message);
            break;
        case 11:
            snprintf(message, sizeof(message),
                     "Сutting one Note and Delay\r\n");
            custom_melody_size = custom_melody_size - 2;
            if (custom_melody_size < 0)
                custom_melody_size = 0;
            sendMessage(message);
            break;
    }
}

void handleInput(int pressed_key) {
    if (pressed_key < 1 || pressed_key > 12)
        return;
    switch (currentMusicBoxStatus) {
        case MUSIC_BOX_PLAYING:
            handleInputPlaying(pressed_key);
            break;
        case MUSIC_BOX_EDITING:
            handleInputEditing(pressed_key);
            break;
        case MUSIC_BOX_EDITING_NOTE:
            handleInputEditingNote(pressed_key);
            break;
        case MUSIC_BOX_EDITING_TEMPO:
            handleInputEditingTempo(pressed_key);
            break;
        case MUSIC_BOX_EDITING_DELAY:
            handleInputEditingDelay(pressed_key);
            break;
    }
}


/* USER CODE BEGIN 0 */



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    /* USER CODE BEGIN 1 */
    HAL_StatusTypeDef isReaded;
    int shortPressTime = 200;
    int isBeingProceed = 0;
    long startTimePressedButton = 0;
    char chtest[] = "F\n\r";
    int melody_tick = 0;
    int new = 1;
    int played = 0;

    currentMelody = melody_AHA;
    currentMelodySize = sizeof(melody_AHA) / sizeof(melody_AHA[0]);
    currentTempo = TEMPO_AHA;
    currentMelodyPosition = 0;
    int cur_melody = 0;
    int cur_pause = 0;
    int tempo = 0;
    int noteDuration = 0;


    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART6_UART_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();
    MX_TIM6_Init();
    /* USER CODE BEGIN 2 */
    sound_driver_init();
    oled_Init();
    oled_Fill(Black);
    oled_UpdateScreen();
    HAL_TIM_Base_Start_IT(&htim6);
//  ks_continue();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
//        blinkGreenDiode();
    	keyboard_read();
//    	delay(10);
        if (!getButtonState()) {
            if (!isBeingProceed) {
                startTimePressedButton = getCurrentTime();
                isBeingProceed = 1;
            }
        } else {
            if (isBeingProceed && getCurrentTime() - startTimePressedButton > shortPressTime) {
                if (mode == 1) {
                    mode = 0;
                    sendMessage(mode0);
                } else {
                    mode = 1;
                    sendMessage(mode1);
                }
                sound_driver_volume_mute();
            }
            isBeingProceed = 0;
        }
        int key_pressed = buffer_read();
//        snprintf(message, sizeof(message),
//                                 "Key beffore pressed is %d\r\n", key_pressed);
//        sendMessage(message);

        if (mode) {
            if (key_pressed > 0 && key_pressed < 13) {
                snprintf(message, sizeof(message),
                         "Key pressed is %d\r\n", key_pressed);
                sendMessage(message);
            }
            continue;
        }
        if (key_pressed > 0 && key_pressed < 13) {
            if (key_pressed >= 1 && key_pressed <= 9) {
                handle_calc(calcChars[key_pressed]);
                continue;
            }
            if (key_pressed == 11) {
                handle_calc(calcChars[0]);
                continue;
            }
            if (key_pressed == 12){
                handle_calc(calcCharsOperations[4]);
                continue;
            }
            nextOperation();
            handle_calc(calcCharsOperations[curCalcsOperation]);

        }
//        sendMessage("KB test complete\r\n");
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
