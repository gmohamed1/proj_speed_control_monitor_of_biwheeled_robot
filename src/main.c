/* Controlling and monitoring the speed of bi-wheeled robot (PIC16F877A)*/

//================ LCD1 (mikroC) =================
sbit LCD_RS at RD0_bit;
sbit LCD_EN at RD1_bit;
sbit LCD_D4 at RD2_bit;
sbit LCD_D5 at RD3_bit;
sbit LCD_D6 at RD4_bit;
sbit LCD_D7 at RD5_bit;

sbit LCD_RS_Direction at TRISD0_bit;
sbit LCD_EN_Direction at TRISD1_bit;
sbit LCD_D4_Direction at TRISD2_bit;
sbit LCD_D5_Direction at TRISD3_bit;
sbit LCD_D6_Direction at TRISD4_bit;
sbit LCD_D7_Direction at TRISD5_bit;

//================ LCD2 (MANUAL) =================
#define LCD2_RS RB0_bit
#define LCD2_EN RB1_bit
#define LCD2_D4 RB4_bit
#define LCD2_D5 RB5_bit
#define LCD2_D6 RB6_bit
#define LCD2_D7 RB7_bit

#define LCD2_RS_DIR TRISB0_bit
#define LCD2_EN_DIR TRISB1_bit
#define LCD2_D4_DIR TRISB4_bit
#define LCD2_D5_DIR TRISB5_bit
#define LCD2_D6_DIR TRISB6_bit
#define LCD2_D7_DIR TRISB7_bit

void LCD2_Pulse(){
  LCD2_EN = 1; Delay_us(5);
  LCD2_EN = 0; Delay_us(5);
}

void LCD2_Send4(char d){
  LCD2_D4 = d & 1;
  LCD2_D5 = (d>>1)&1;
  LCD2_D6 = (d>>2)&1;
  LCD2_D7 = (d>>3)&1;
  LCD2_Pulse();
}

void LCD2_Cmd(char cmd){
  LCD2_RS = 0;
  LCD2_Send4(cmd>>4);
  LCD2_Send4(cmd);
  Delay_ms(2);
}

void LCD2_Chr(char row, char col, char c){
  char pos;
  if(row==1) pos = 0x80 + (col-1);
  else pos = 0xC0 + (col-1);
  LCD2_Cmd(pos);
  LCD2_RS = 1;
  LCD2_Send4(c>>4);
  LCD2_Send4(c);
}

void LCD2_Out(char row, char col, char *txt){
  while(*txt){
    LCD2_Chr(row,col++,*txt++);
  }
}

void LCD2_Init(){
  LCD2_RS_DIR = LCD2_EN_DIR = 0;
  LCD2_D4_DIR = LCD2_D5_DIR = LCD2_D6_DIR = LCD2_D7_DIR = 0;
  Delay_ms(20);
  LCD2_Cmd(0x33);
  LCD2_Cmd(0x32);
  LCD2_Cmd(0x28);
  LCD2_Cmd(0x0C);
  LCD2_Cmd(0x06);
  LCD2_Cmd(0x01);
}

//================ MAIN =================
#define DEADZONE 10

unsigned int adcA, adcB;
unsigned int pwmA, pwmB;
char txt[16];

// Motor pins
#define M1_IN1 RC0_bit
#define M1_IN2 RC3_bit

#define M2_IN1 RC4_bit
#define M2_IN2 RC5_bit

void main(){

  ADCON1 = 0x80;   // AN0, AN1 analog

  // Directions
  TRISC0_bit = 0;
  TRISC3_bit = 0;
  TRISC4_bit = 0;
  TRISC5_bit = 0;

  // PWM pins
  TRISC1_bit = 0;   // PWM1
  TRISB3_bit = 0;   // PWM2

  // LCD init
  Lcd_Init();
  Lcd_Cmd(_LCD_CLEAR);
  Lcd_Cmd(_LCD_CURSOR_OFF);
  LCD2_Init();

  // PWM init
  PWM1_Init(5000);
  PWM2_Init(5000);
  PWM1_Start();
  PWM2_Start();

  while(1){

    adcA = ADC_Read(0);   // Motor 1 pot
    adcB = ADC_Read(1);   // Motor 2 pot

    //=========== MOTOR 1 ===========
    if(adcA > 512 + DEADZONE){
      M1_IN1 = 1; M1_IN2 = 0;
      pwmA = (adcA - 512) * 2;
      Lcd_Out(1,1,"M1 FORWARD ");
    }
    else if(adcA < 512 - DEADZONE){
      M1_IN1 = 0; M1_IN2 = 1;
      pwmA = (512 - adcA) * 2;
      Lcd_Out(1,1,"M1 REVERSE ");
    }
    else{
      M1_IN1 = 0; M1_IN2 = 0;
      pwmA = 0;
      Lcd_Out(1,1,"M1 STOP    ");
    }

    if(pwmA > 1023) pwmA = 1023;
    PWM1_Set_Duty(pwmA);

    //=========== MOTOR 2 ===========
    if(adcB > 512 + DEADZONE){
      M2_IN1 = 1; M2_IN2 = 0;
      pwmB = (adcB - 512) * 2;
      LCD2_Out(1,1,"M2 FORWARD ");
    }
    else if(adcB < 512 - DEADZONE){
      M2_IN1 = 0; M2_IN2 = 1;
      pwmB = (512 - adcB) * 2;
      LCD2_Out(1,1,"M2 REVERSE ");
    }
    else{
      M2_IN1 = 0; M2_IN2 = 0;
      pwmB = 0;
      LCD2_Out(1,1,"M2 STOP    ");
    }

    if(pwmB > 1023) pwmB = 1023;
    PWM2_Set_Duty(pwmB);

    //=========== Display Speed ===========
    WordToStr(pwmA, txt);
    Lcd_Out(2,1,"SPD:");
    Lcd_Out(2,5,txt);

    WordToStr(pwmB, txt);
    LCD2_Out(2,1,"SPD:");
    LCD2_Out(2,5,txt);

    Delay_ms(150);
  }
}