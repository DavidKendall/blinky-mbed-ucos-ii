#include <stdbool.h>
#include <ucos_ii.h>
#include <mbed.h>
#include "C12832.h"
#include "MMA7660.h"
#include "LM75B.h"
#include <stdio.h>

/*
*********************************************************************************************************
*                                            APPLICATION TASK PRIORITIES
*********************************************************************************************************
*/

typedef enum {
  SAMPLE_JOYSTICK_PRIO = 4,
  SAMPLE_SW3_PRIO,
  SAMPLE_POT_PRIO,
  SAMPLE_ACCEL_PRIO,
  UPDATE_LCD_PRIO,
  UPDATE_SPEAKER_PRIO,
  LED_RED_PRIO,
  LED_GREEN_PRIO,
  SAMPLE_TEMP_PRIO,
} taskPriorities_t;

/*
*********************************************************************************************************
*                                            APPLICATION TASK STACKS
*********************************************************************************************************
*/

#define  LED_RED_STK_SIZE              256
#define  LED_GREEN_STK_SIZE            256
#define  SAMPLE_POT_STK_SIZE           256
#define  SAMPLE_JOYSTICK_STK_SIZE      256
#define  SAMPLE_SW3_STK_SIZE           256
#define  SAMPLE_ACCEL_STK_SIZE         256
#define  SAMPLE_TEMP_STK_SIZE          256
#define  UPDATE_LCD_STK_SIZE           256
#define  UPDATE_SPEAKER_STK_SIZE       256

static OS_STK ledRedStk[LED_RED_STK_SIZE];
static OS_STK ledGreenStk[LED_GREEN_STK_SIZE];
static OS_STK samplePotStk[SAMPLE_POT_STK_SIZE];
static OS_STK sampleJoystickStk[SAMPLE_JOYSTICK_STK_SIZE];
static OS_STK sampleSW3Stk[SAMPLE_SW3_STK_SIZE];
static OS_STK sampleAccelStk[SAMPLE_ACCEL_STK_SIZE];
static OS_STK sampleTempStk[SAMPLE_TEMP_STK_SIZE];
static OS_STK updateLCDStk[UPDATE_LCD_STK_SIZE];
static OS_STK updateSpeakerStk[UPDATE_SPEAKER_STK_SIZE];

/*
*********************************************************************************************************
*                                            APPLICATION FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void appTaskLedRed(void *pdata);
static void appTaskLedGreen(void *pdata);
static void samplePot(void *pdata);
static void sampleJoystick(void *pdata);
static void sampleSW3(void *pdata);
static void sampleAccel(void *pdata);
static void sampleTemp(void *pdata);
static void updateLCD(void *pdata);
static void updateSpeaker(void *pdata);

static void ledToggle(DigitalOut led);
/*
*********************************************************************************************************
*                                            GLOBAL TYPES AND VARIABLES 
*********************************************************************************************************
*/

static C12832 lcd(D11, D13, D12, D7, D10);

static float pot1Val = 0.0f;
static float pot2Val = 0.0f;
static char joystickVal = '\0';
static float accelVal[3] = {0.0f, 0.0f, 0.0f};
static float tempVal = 0.0f;
static bool sw3Pressed = false;

/*
*********************************************************************************************************
*                                            GLOBAL FUNCTION DEFINITIONS
*********************************************************************************************************
*/

int main() {

  /* Initialise the OS */
  OSInit();                                                   

  /* Create the tasks */
  OSTaskCreate(appTaskLedRed,                               
               (void *)0,
               (OS_STK *)&ledRedStk[LED_RED_STK_SIZE - 1],
               LED_RED_PRIO);
  
  OSTaskCreate(appTaskLedGreen,                               
               (void *)0,
               (OS_STK *)&ledGreenStk[LED_GREEN_STK_SIZE - 1],
               LED_GREEN_PRIO);

  OSTaskCreate(samplePot,                               
               (void *)0,
               (OS_STK *)&samplePotStk[SAMPLE_POT_STK_SIZE - 1],
               SAMPLE_POT_PRIO);

  OSTaskCreate(sampleJoystick,                               
               (void *)0,
               (OS_STK *)&sampleJoystickStk[SAMPLE_JOYSTICK_STK_SIZE - 1],
               SAMPLE_JOYSTICK_PRIO);

  OSTaskCreate(sampleSW3,                               
               (void *)0,
               (OS_STK *)&sampleSW3Stk[SAMPLE_SW3_STK_SIZE - 1],
               SAMPLE_SW3_PRIO);

  OSTaskCreate(sampleAccel,                               
               (void *)0,
               (OS_STK *)&sampleAccelStk[SAMPLE_ACCEL_STK_SIZE - 1],
               SAMPLE_ACCEL_PRIO);

  OSTaskCreate(sampleTemp,                               
               (void *)0,
               (OS_STK *)&sampleTempStk[SAMPLE_TEMP_STK_SIZE - 1],
               SAMPLE_TEMP_PRIO);

  OSTaskCreate(updateLCD,                               
               (void *)0,
               (OS_STK *)&updateLCDStk[UPDATE_LCD_STK_SIZE - 1],
               UPDATE_LCD_PRIO);
  
  OSTaskCreate(updateSpeaker,                               
               (void *)0,
               (OS_STK *)&updateSpeakerStk[UPDATE_SPEAKER_STK_SIZE - 1],
               UPDATE_SPEAKER_PRIO);
  
  /* Start the OS */
  OSStart();                                                  
  
  /* Should never arrive here */ 
  return 0;      
}

/*
*********************************************************************************************************
*                                            APPLICATION TASK DEFINITIONS
*********************************************************************************************************
*/

static void appTaskLedRed(void *pdata) {
  DigitalOut red(LED_RED);
  
  red = 1;

  /* Task main loop */
  while (true) {
    ledToggle(red);
    OSTimeDlyHMSM(0,0,0,500);
  }
}

static void appTaskLedGreen(void *pdata) {
  DigitalOut green(LED_GREEN);

  green = 0;
  while (true) {
    ledToggle(green);	
    OSTimeDlyHMSM(0,0,0,500);
  } 
}

static void samplePot(void *pdata) {
  AnalogIn pot1(A0); // Pot 1 - Left
  AnalogIn pot2(A1); // Pot 2 - Right

  while (true) {
    pot1Val = pot1.read();
    pot2Val = pot2.read();
    OSTimeDlyHMSM(0,0,0,100);
  } 
}

static void sampleJoystick(void *pdata) {
  static DigitalIn joystick[5] = {D4, A2, A3, A4, A5};
  static char symbols[] = {'C', 'U', 'D', 'L', 'R'};
  int i = 0;

  /* Start the OS ticker -- must be done in the highest priority task */
  SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);

  while (true) {
    joystickVal = '-';
    for (i = 0; i < 5; i+=1) {
        if (joystick[i] == 1) {
            joystickVal = symbols[i];
            break;
        }
     }
    OSTimeDlyHMSM(0,0,0,100);
  } 
}

static void sampleSW3(void *pdata) {
  static DigitalIn sw3(PTA4);

  while (true) {
    sw3Pressed = (sw3 == 0) ? true : false;
    OSTimeDlyHMSM(0,0,0,100);
  } 
}

static void sampleAccel(void *pdata) {
  static MMA7660 accel(D14, D15);

  while (true) {
    accelVal[0] = accel.x();
    accelVal[1] = accel.y();
    accelVal[2] = accel.z();
    OSTimeDlyHMSM(0,0,0,100);
  } 
}

static void sampleTemp(void *pdata) {
  static LM75B temp(D14, D15);

  while (true) {
    tempVal = temp.read();
    OSTimeDlyHMSM(0,0,1,0);
  } 
}

static void updateLCD(void *pdata) {

  lcd.cls();
  while (true) {
    lcd.locate(0, 0);
    lcd.printf("L: %0.2f", pot1Val);
    lcd.locate(0, 8);
    lcd.printf("R: %0.2f", pot2Val);
    lcd.locate(0, 16);
    lcd.printf("J: %c", joystickVal);
    lcd.locate(43, 0);
    lcd.printf("X: %0.2f", accelVal[0]);
    lcd.locate(43, 8);
    lcd.printf("Y: %0.2f", accelVal[1]);
    lcd.locate(43, 16);
    lcd.printf("Z: %0.2f", accelVal[2]);
    lcd.locate(86, 0);
    lcd.printf("T: %02.2f", tempVal);
    OSTimeDlyHMSM(0,0,0,100);
  } 
}

static void updateSpeaker(void *pdata) {
  static PwmOut speaker(D6);

  speaker.period_us(2272);
  speaker.pulsewidth_us(0);
  while (true) {
    if (sw3Pressed) {
      speaker.pulsewidth_us(1136);
    } 
    else {
      speaker.pulsewidth_us(0);
    }
    OSTimeDlyHMSM(0,0,0,100);
  } 
}

static void ledToggle(DigitalOut led) {
  led = 1 - led;
}
