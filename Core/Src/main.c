/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "fatfs_sd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KIRI 3
#define TENGAH 4
#define KANAN 5
#define DEFAULT 99
#define OK 15
#define CLR 14
#define DEL 13

#define STEPEDIT 999999
#define STEPERROR 999998
#define STEPDELETE 999997

#define LEDFALSE_PORT GPIOB
#define LEDFALSE_PIN GPIO_PIN_1 
#define LEDTRUE_PORT GPIOB
#define LEDTRUE_PIN GPIO_PIN_0 
#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_PIN_11 

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    //if(GPIO_Pin == GPIO_PIN_10) 
			//NVIC_SystemReset();
}

/*
	PINOUT 
	RX - A10
	TX - A9
	readA - A0
	readB - A1
	readC - A2
	readQ - A3
	LED FALSE - B1
	LED TRUE - B0
	BUZZER - B11
	RESET - B10
*/

uint8_t FOOTER[30] = {0xFF,0xFF,0xFF};
uint8_t Rx_Data[7];
int step = 0, stepQuestion = 0, stepEdit = 0, stepAdd = 0, stepDelete;
int button = 99;
int counter;
int correctAnswer;
int lastTBuzz;

int readA,readB,readC,readQ;
int answer;

//========== ADDING QUESTION ==========//
char buffA,buffB,buffC,buffQ;
char bufferQuestion[100];
int cntKey = 0;
int questionType;

//========== TAKING QUESTION ==========//
char questionList[100][30];
char inputList[100][30];
int inA[100],inB[100],inC[100],inQ[100],qType[100];
int j,totalQuestion,pointerQ,stepQ;
char data[10][30];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
char inputKey(){
	if (button != 99){
		if (button == 3){
			button = 99;
			return '+';
		}
		if (button == 6){
			button = 99;
			return '*';
		}
		if (button == 7){
			button = 99;
			return '\'';
		}
		if (button == 8){
			button = 99;
			return '=';
		}
		if (button == 9){
			button = 99;
			return 'Q';
		}
		if (button == 10){
			button = 99;
			return 'A';
		}
		if (button == 11){
			button = 99;
			return 'B';
		}
		if (button == 12){
			button = 99;
			return 'C';
		}
		if (button == 19){
			button = 99;
			return '1';
		}
		if (button == 18){
			button = 99;
			return '0';
		}
		if (button == 17){
			button = 99;
			return '(';
		}
		if (button == 16){
			button = 99;
			return ')';
		}
		if (button == 13){
			button = 99;
			return 13;
		}
		if (button == 14){
			button = 99;
			return 14;
		}
		if (button == 15){
			button = 99;
			return 15;
		}
		else {
			button = 99;
			return 99;
		}
	}
	return 99;
}

void HMISendText(char* ID, char* string){
	uint8_t buf[50];
	int len = sprintf((char *)buf,"%s.txt=\"%s\"", ID, string);
	HAL_UART_Transmit(&huart1, buf, len, 100);	//Kirim TX
	HAL_UART_Transmit(&huart1, FOOTER, 3, 100);	//Kirim TX
}

void HMISend(char* string){
	uint8_t buf[50];
	int len = sprintf((char *)buf,"%s", string);
	HAL_UART_Transmit(&huart1, buf, len, 100);	//Kirim TX
	HAL_UART_Transmit(&huart1, FOOTER, 3, 100);	//Kirim TX
}

void HMIClearText(){
	HMISendText("t0","");
	HMISendText("t1","");
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART1) {  //current UART
		button = Rx_Data[2];
		HAL_UART_Receive_IT(huart, Rx_Data, 7);
	}
}

void readInput(){
		readA = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
		readB = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1);
		readC = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2);
		readQ = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3);
}

uint8_t qAB(int desA, int desB, int desQ){
	if (readA == desA && readB == desB && readQ == desQ)
		return 1;
	else 
		return 0;
}

uint8_t qABC(int desA, int desB, int desC, int desQ){
	if (readA == desA && readB == desB && readC == desC && readQ == desQ)
		return 1;
	else 
		return 0;
}

void displayqAB(int A, int B, int Q, char* question, char* input){
	if (step == 0){
		step++;
	}
	else if (step == 1) {
			readInput();
			HMISendText("t0",question);
			HMISendText("t1",input);
			HMISendText("b1","Answer");
			if (button == 4){
				 button = DEFAULT;
				 answer = qAB(A,B,Q);
				 HMIClearText();
				 if (answer){
					 HMISendText("t0","Correct Answer");
					 correctAnswer++;
					 HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_RESET);
					 HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_SET);
					 HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
				 }
				 else {
					 HMISendText("t0","Incorrect Answer");
					 HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_SET);
					 HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_RESET);
					 lastTBuzz = HAL_GetTick();
				 }
				 HMISendText("b1","Next");
				 step++;
			}
		}
		else if (step == 2){
			if (HAL_GetTick() - lastTBuzz < 1000)
				HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_SET);
			else HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
			
			if (button == 4){
				HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
				HMIClearText();
				button = DEFAULT;
				pointerQ++;
				step = 0;
			}
		}
}

void displayqABC(int A, int B, int C, int Q, char* question, char* input){
		if (step == 0){
			step++;
		}
		else if (step == 1) {
			readInput();
			HMISendText("t0",question);
			HMISendText("t1",input);
			HMISendText("b1","Answer");
			if (button == 4){
				 button = DEFAULT;
				 answer = qABC(A,B,C,Q);
				 HMIClearText();
				 if (answer){
					 HMISendText("t0","Correct Answer");
					 correctAnswer++;
					 HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_RESET);
					 HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_SET);
					 HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
				 }
				 else {
					 HMISendText("t0","Incorrect Answer");
					 HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_SET);
					 HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_RESET);
					 lastTBuzz = HAL_GetTick();
				 }
				 HMISendText("b1","Next");
				 step++;
			}
		}
		else if (step == 2){
			if (HAL_GetTick() - lastTBuzz < 1000)
				HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_SET);
			else HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
			
			if (button == 4){
				HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
				HMIClearText();
				button = DEFAULT;
				pointerQ++;
				step = 0;
			}
		}
}

void displayScore(){
	if (step == 0){
		char buf[50];
		sprintf(buf,"Score = %d",correctAnswer);
		HMISendText("t0",buf);
		sprintf(buf,"Total %d Question",totalQuestion);
		HMISendText("t1",buf);
		HMISendText("b1","Restart");
		step++;
	}
	else if (step == 1){
		if (button == 4){
				button = DEFAULT;
				HMISend("vis b0,1");
				HMISend("vis b1,1");
				HMISend("vis b2,0");
				HMISendText("t0","Press Start to Begin");
				HMISendText("t1","");
				HMISendText("b0","Edit");
				HMISendText("b1","Start");
				correctAnswer = 0;
				stepQuestion= 0;
				step = 0;
			}
	}
}


int SPlast, SPnow, SLen;
uint8_t flagSDError;

FATFS fs;  // file system
FIL fil; // File
FILINFO fno;
FRESULT fresult,fmount;  // result
UINT br, bw;  // File read/write count

/**** capacity related *****/
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];  // to store strings..
char bufferX[100];

int bufsize (char *buf)
{
	int i=0;
	while (*buf++ != '\0') i++;
	return i;
}

void clear_buffer (void)
{
	for (int i=0; i<BUFFER_SIZE; i++) buffer[i] = '\0';
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	HAL_Delay(1000);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(50);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_SET);
	HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_SET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(LEDFALSE_PORT,LEDFALSE_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LEDTRUE_PORT,LEDTRUE_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
	HAL_Delay(200);

	//HAL_Delay(3000);
	//HAL_Delay(4000);
	HMISend("page 0");
	
	HMISend("vis b0,0");
	HMISend("vis b1,0");
	HMISend("vis b2,0");
	HMISendText("t0","Mounting SD Card...");
	HAL_Delay(100);

	fresult = f_mount(&fs, "/", 1); //fatfs

	//== Test Drive Delete ==//
	/*
	int SPlast, SPnow, SLen;
	
	fresult = f_open(&fil, "testdrive.txt", FA_READ | FA_OPEN_ALWAYS | FA_WRITE);		
	
	//Pada Pointer 0, Tidak Membaca soal
	
	SPlast = f_tell(&fil);
	f_gets(buffer, BUFFER_SIZE, &fil);	
	SPnow = f_tell(&fil);
	
	//Pada Pointer 1, Membaca Soal 1
	SPlast = f_tell(&fil);
	f_gets(buffer, BUFFER_SIZE, &fil);	
	SPnow = f_tell(&fil);
	
	
	//Delete Happening	
	f_read(&fil,buffer,1000,&br);
	f_lseek(&fil,SPlast);
	f_puts(buffer,&fil);
	f_truncate(&fil);
	
	f_close(&fil);
	HAL_Delay(100000);
	*/
	//======================//

	if (fresult != FR_OK){
		HMISendText("t0","SD CARD Cant Detect");
	}
	else {
    fresult = f_open(&fil, "soal.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		if (fresult == FR_OK){
			HMISendText("t0","SD CARD Mounted...");
			fmount = fresult;	
			f_close(&fil);
		}
		else if (fresult != FR_OK){
			HMISendText("t0","Cant Open File...");
		}
	}
	
	if (fresult == FR_OK){
		HMISend("vis b0,1");
		HMISend("vis b1,1");
		HMISend("vis b2,0");
		HMISendText("t0","Press Start to Begin");
		HMISendText("t1","");
		HMISendText("b0","Edit");
		HMISendText("b1","Start");	
	}
	else {
		HMISend("vis b0,0");
		HMISend("vis b1,1");
		HMISend("vis b2,0");
		HMISendText("t1","");
		HMISendText("b1","Restart");	
		stepQuestion = STEPERROR;
	}
	HAL_UART_Abort_IT(&huart1);
	HAL_UART_Receive_IT(&huart1,Rx_Data,7);
	HAL_Delay(100);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3) == GPIO_PIN_SET)
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);
		else HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_RESET);
		
		if (stepQuestion == 0){ //Inisialisasi
			if (button == KIRI){
				button = 99;
				HMISend("vis b0,1");
				HMISend("vis b1,1");
				HMISend("vis b2,1");
				HMISendText("t0","Editor Menu");
				HMISendText("t1","");
				HMISendText("b0","Back");
				HMISendText("b1","CLR SDCARD");
				HMISendText("b2","Add QST");
				stepQuestion = STEPEDIT;
				fresult = f_open(&fil, "soal.txt", FA_READ | FA_OPEN_ALWAYS | FA_WRITE);			
			}
			else if (button == TENGAH){ //startQuestion
				button = 99;
				stepQuestion = 1;
				totalQuestion = 0;
				pointerQ = 0;
				for (int i=0; i<sizeof(inA)/4; ++i){
					inA[i] = 0; 
					inB[i] = 0; 			
					inC[i] = 0; 			
					inQ[i] = 0; 			
					qType[i] = 0; 			
				}
				for (int i=0;i<sizeof(questionList)/4/30;i++)
					memset(questionList[i],0,strlen(questionList[i]));
				for (int i=0;i<sizeof(inputList)/4/30;i++)
					memset(inputList[i],0,strlen(inputList[i]));
				fresult = f_open(&fil, "soal.txt", FA_READ | FA_OPEN_ALWAYS);

				HMISend("vis b0,0");
				HMISend("vis b1,1");
				HMISend("vis b2,0");
				HMISendText("t0","Total Question");
				HMISendText("t1","");
				HMISendText("b1","Start");
			}
		}
		else if (stepQuestion == 1) {
				f_gets(buffer, BUFFER_SIZE, &fil);
				for (int i=0;i<sizeof(data)/4/30;i++)
					memset(data[i],0,strlen(data[i]));
				j = 0;
				for(int i=0;i<strlen(buffer);i++){
					if (buffer[i] == ';') {
						j++;
						sprintf(data[j],"");
					}
					else {
						sprintf(data[j],"%s%c",data[j],buffer[i]);
						//strcat(data[j], &buffer[i]);
					}
				}
				sprintf(bufferX,"%d",atoi(data[0]));
				HMISendText("t1",bufferX);
				
				qType[totalQuestion] = atoi(data[0]);
				if (qType[totalQuestion] == 0) { //Untuk Tipe AB
					inA[totalQuestion] = atoi(data[1]);
					inB[totalQuestion] = atoi(data[2]);
					inC[totalQuestion] = 0;
					inQ[totalQuestion] = atoi(data[3]);
					sprintf(questionList[totalQuestion],"%s",data[4]);
					sprintf(inputList[totalQuestion],"%s",data[5]);
				}
				else if (qType[totalQuestion] == 1) { //Untuk Tipe ABC
						inA[totalQuestion] = atoi(data[1]);
						inB[totalQuestion] = atoi(data[2]);
						inC[totalQuestion] = atoi(data[3]);
						inQ[totalQuestion] = atoi(data[4]);
						sprintf(questionList[totalQuestion],"%s",data[5]);
						sprintf(inputList[totalQuestion],"%s",data[6]);
				}			
				if (f_size(&fil) != 0)
					totalQuestion++;
				
				if (f_eof(&fil) != 0) {
					sprintf(bufferX,"%d",totalQuestion);
					f_close(&fil);
					if (totalQuestion == 0) {
						stepQuestion = 0;
						HMISend("vis b0,1");
						HMISend("vis b1,1");
						HMISend("vis b2,0");
						HMISendText("t0","Cant Start");
						HMISendText("t1","Please Input Question");
						HMISendText("b0","Edit");
						HMISendText("b1","Start");
					}
					else{
						stepQuestion = 2;
						HMISendText("t1",bufferX);
						HMISendText("b1","Start");
					}
				}			
		}
		else if (stepQuestion == 2){
			if (button == TENGAH){
				button = 99;
				pointerQ = 0;
				stepQ = 0;
				stepQuestion = 3;
			}
		}
		else if (stepQuestion == 3){
			if (pointerQ == totalQuestion)
				displayScore();
			else {
				if (qType[pointerQ] == 0)
					displayqAB(inA[pointerQ],inB[pointerQ],inQ[pointerQ],questionList[pointerQ],inputList[pointerQ]);
				else if (qType[pointerQ] == 1)
					displayqABC(inA[pointerQ],inB[pointerQ],inC[pointerQ],inQ[pointerQ],questionList[pointerQ],inputList[pointerQ]);
			}
		}
		else if (stepQuestion == STEPEDIT){
			if (stepEdit == 0) { //INIT
				if (button == KIRI) { //Back
					button = 99;
					stepQuestion = 0;
					f_close(&fil);
					
					HMISend("vis b0,1");
					HMISend("vis b1,1");
					HMISend("vis b2,0");
					HMISendText("t0","Press Start to Begin");
					HMISendText("t1","");
					HMISendText("b0","Edit");
					HMISendText("b1","Start");
				}
				else if (button == TENGAH) { //Clear SD
					button = 99;
					stepEdit = 0;
					stepDelete = 0;
					stepQuestion = STEPDELETE;
					
					HMISend("vis b0,1");
					HMISend("vis b1,1");
					HMISend("vis b2,1");
					HMISendText("t0","Choose Delete Mode");
					HMISendText("t1","");
					HMISendText("b0","Cancel");
					HMISendText("b1","ClearOne");
					HMISendText("b2","ClearAll");
				}	
				else if (button == KANAN) { //Add SD
					button = 99;
					stepEdit = 1;
					HMISend("vis b0,1");
					HMISend("vis b1,1");
					HMISend("vis b2,1");
					HMISendText("t0","Choose Question Type");
					HMISendText("t1","");
					HMISendText("b0","Back");
					HMISendText("b1","AB");
					HMISendText("b2","ABC");
				}	
			}
			else if (stepEdit == 1){//ADD Question
				if (button == KIRI){
					stepEdit = 0;
					button = 99;
					HMISend("vis b0,1");
					HMISend("vis b1,1");
					HMISend("vis b2,1");
					HMISendText("t0","Editor Menu");
					HMISendText("t1","");
					HMISendText("b0","Back");
					HMISendText("b1","CLR SDCARD");
					HMISendText("b2","Add QST");
				}
				else if (button == TENGAH){
					stepEdit = 2;
					button = 99;
					questionType = 0;
					HMISend("vis b0,0");
					HMISend("vis b1,1");
					HMISend("vis b2,0");
					HMISendText("t0","Input Question String");
					HMISendText("t1","Ex: Q=A+B");
					HMISendText("b1","Next");
				}
				else if (button == KANAN){
					stepEdit = 2;
					button = 99;
					questionType = 1;
					//HMISend("page 2");
					//HMISendText("t0","Input Question String");
					HMISend("vis b0,0");
					HMISend("vis b1,1");
					HMISend("vis b2,0");
					HMISendText("t0","Input Question String");
					HMISendText("t1","Ex: Q=A+(B*C)'");
					HMISendText("b1","Next");
				}
			}
			else if (stepEdit == 2){
			 if (button == TENGAH){
					stepEdit = 3;
					button = 99;
					HMISend("page 2");
					HMISendText("t0","Input Question String");
					memset(bufferQuestion,0,strlen(bufferQuestion));
					cntKey = 0;
				}
			}
			else if (stepEdit == 3){
				char key = inputKey();
				if (key != 99 && key != CLR && key != DEL && key != OK){
					bufferQuestion[cntKey] = key;
					cntKey++;
				}
				if (key == CLR){
					cntKey = 0;
					memset(bufferQuestion,0,strlen(bufferQuestion));
				}
				if (key == DEL){
					if (cntKey != 0)
						cntKey--;
					bufferQuestion[cntKey] = 0;
				}
				if (key != 99)
					HMISendText("t1",bufferQuestion);
				if (key == OK){
					HMISendText("t0","Input A Value (0/1)");
					HMISendText("t1","0");
					buffA = 0;
					stepEdit = 4;
				}
			}
			else if (stepEdit == 4){
				char key = inputKey();
				if (key == '1'){
					buffA = 1;
					HMISendText("t1","1");
				}
				if (key == '0'){
					buffA = 0;
					HMISendText("t1","0");
				}
				if (key == OK){
					HMISendText("t0","Input B Value (0/1)");
					HMISendText("t1","0");
					buffB = 0;
					stepEdit = 5;
				}
			}
			else if (stepEdit == 5){
				char key = inputKey();
				if (key == '1'){
					buffB = 1;
					HMISendText("t1","1");
				}
				if (key == '0'){
					buffB = 0;
					HMISendText("t1","0");
				}
				if (key == OK){
					if (questionType == 0){
						HMISendText("t0","Input Q Value (0/1)");
						HMISendText("t1","0");
						stepEdit = 7;
					}
					else if (questionType == 1){
						HMISendText("t0","Input C Value (0/1)");
						HMISendText("t1","0");
						stepEdit = 6;
						buffC = 0;
					}
				}
			}
			else if (stepEdit == 6){
				char key = inputKey();
				if (key == '1'){
					buffC = 1;
					HMISendText("t1","1");
				}
				if (key == '0'){
					buffC = 0;
					HMISendText("t1","0");
				}
				if (key == OK){
					HMISendText("t0","Input Q Value (0/1)");
					HMISendText("t1","0");
					buffQ = 0;
					stepEdit = 7;
				}
			}
			else if (stepEdit == 7){
				char key = inputKey();
				if (key == '1'){
					buffQ = 1;
					HMISendText("t1","1");
				}
				if (key == '0'){
					buffQ = 0;
					HMISendText("t1","0");
				}
				if (key == OK){
					HMISendText("t0","Review the Question");
					HMISendText("t1","Press OK to Continue");
					stepEdit = 8;
				}
			}
			else if (stepEdit == 8){
				char key = inputKey();
				if (key == OK){
					if (questionType == 0)
						sprintf(bufferX,"A=%d,B=%d,Q=%d",buffA,buffB,buffQ);
					else
						sprintf(bufferX,"A=%d,B=%d,C=%d,Q=%d",buffA,buffB,buffC,buffQ);
					HMISendText("t0",bufferQuestion);
					HAL_Delay(5);
					HMISendText("t1",bufferX);
					stepEdit = 9;
				}
			}
			else if (stepEdit == 9){
				char key = inputKey();
				if (key == OK){
					fresult = f_lseek(&fil, f_size(&fil));
					
					if (questionType == 0){ //0;1;1;1;Q=A+B;A=0,B=0;
						if ((f_size(&fil) != 0))
							sprintf(buffer,"\n%d;%d;%d;%d;%s;A=%d,B=%d;",questionType,buffA,buffB,buffQ,bufferQuestion,buffA,buffB);
						else sprintf(buffer,"%d;%d;%d;%d;%s;A=%d,B=%d;",questionType,buffA,buffB,buffQ,bufferQuestion,buffA,buffB);
					}
					else if (questionType == 1){
						if ((f_size(&fil) != 0))
							sprintf(buffer,"\n%d;%d;%d;%d;%d;%s;A=%d,B=%d,C=%d;",questionType,buffA,buffB,buffC,buffQ,bufferQuestion,buffA,buffB,buffC);
						else sprintf(buffer,"%d;%d;%d;%d;%d;%s;A=%d,B=%d,C=%d;",questionType,buffA,buffB,buffC,buffQ,bufferQuestion,buffA,buffB,buffC);
					}
					
					fresult = f_write(&fil, buffer, bufsize(buffer), &bw);
					if (fresult == FR_OK){
						HMISendText("t0","Done Writing to SDCard");
						HMISendText("t1",buffer);
						f_close(&fil);
						fresult = f_open(&fil, "soal.txt", FA_READ | FA_OPEN_ALWAYS | FA_WRITE);
					}
					else {
						HMISendText("t0","Cant Write to SDCard");
						HMISendText("t1","Press OK to Restart");
					}
					stepEdit = 10;
				}
			}
			else if (stepEdit == 10){
				char key = inputKey();
				if (key == OK){
					if (fresult == FR_OK){
						HMISend("page 0");
						HMISend("vis b1,1");
						HMISend("vis b2,1");
						HMISend("vis b2,1");
						HMISendText("t0","Editor Menu");
						HMISendText("t1","");
						HMISendText("b0","Back");
						HMISendText("b1","Clear QST");
						HMISendText("b2","Add QST");
						stepEdit = 0;
					}
					else NVIC_SystemReset();
				}
			} 
		}
		else if (stepQuestion == STEPDELETE){
			if (stepDelete == 0){
				if (button == KIRI) { //Back
						button = 99;
						stepQuestion = 0;
						f_close(&fil);
						
						HMISend("vis b0,1");
						HMISend("vis b1,1");
						HMISend("vis b2,0");
						HMISendText("t0","Press Start to Begin");
						HMISendText("t1","");
						HMISendText("b0","Edit");
						HMISendText("b1","Start");
					}
				else if (button == TENGAH){
						button = 99;
						if (f_eof(&fil) == 1){
							stepQuestion = 0;
							f_close(&fil);
							
							HMISend("vis b0,1");
							HMISend("vis b1,1");
							HMISend("vis b2,0");
							HMISendText("t0","File Empty");
							HMISendText("t1","");
							HMISendText("b0","Edit");
							HMISendText("b1","Start");	
						}
						else {
							stepDelete = 1;
							SPlast = f_tell(&fil);
							f_gets(buffer, BUFFER_SIZE, &fil);	
							SPnow = f_tell(&fil);
							
							for (int i=0;i<sizeof(data)/4/30;i++)
								memset(data[i],0,strlen(data[i]));
							j = 0;
							for(int i=0;i<strlen(buffer);i++){
								if (buffer[i] == ';') {
									j++;
									sprintf(data[j],"");
								}
								else {
									sprintf(data[j],"%s%c",data[j],buffer[i]);
									//strcat(data[j], &buffer[i]);
								}
							}
							HMISend("vis b0,1");
							HMISend("vis b1,1");
							HMISend("vis b2,1");
							
							HMISendText("t0",data[4]);
							HMISendText("t1",data[5]);
							
							HMISendText("b0","Cancel");
							HMISendText("b1","Next");
							HMISendText("b2","Clear");
						}
				}
				else if (button == KANAN){
						button = 99;
						stepQuestion = 0;
						
						f_close(&fil);
						f_unlink("soal.txt");
						HMISend("vis b0,1");
						HMISend("vis b1,1");
						HMISend("vis b2,0");
						HMISendText("t0","SD Card Cleared");
						HMISendText("t1","Please Input Question");
						HMISendText("b0","Edit");
						HMISendText("b1","Start");
				}
			}
			else if (stepDelete == 1){
				if (button == KIRI) { //Back
						button = 99;
						stepDelete = 0;
						stepQuestion = 0;
						f_close(&fil);
						
						HMISend("vis b0,1");
						HMISend("vis b1,1");
						HMISend("vis b2,0");
						HMISendText("t0","Press Start to Begin");
						HMISendText("t1","");
						HMISendText("b0","Edit");
						HMISendText("b1","Start");
				}
				else if (button == TENGAH){
					button = 99;
					stepDelete = 1;
					SPlast = f_tell(&fil);
					f_gets(buffer, BUFFER_SIZE, &fil);	
					SPnow = f_tell(&fil);
					
					if (strlen(buffer) != 0) {	
						for (int i=0;i<sizeof(data)/4/30;i++)
							memset(data[i],0,strlen(data[i]));
						j = 0;
						for(int i=0;i<strlen(buffer);i++){
							if (buffer[i] == ';') {
								j++;
								sprintf(data[j],"");
							}
							else {
								sprintf(data[j],"%s%c",data[j],buffer[i]);
								//strcat(data[j], &buffer[i]);
							}
						}
						HMISend("vis b0,1");
						HMISend("vis b1,1");
						HMISend("vis b2,1");
						
						HMISendText("t0",data[4]);
						HMISendText("t1",data[5]);

						HMISendText("b0","Cancel");
						HMISendText("b1","Next");
						HMISendText("b2","Clear");
					}
					else {
						stepDelete = 0;
						stepQuestion = 0;
						f_close(&fil);
						
						HMISend("vis b0,1");
						HMISend("vis b1,1");
						HMISend("vis b2,0");
						HMISendText("t0","Press Start to Begin");
						HMISendText("t1","End of Question List");
						HMISendText("b0","Edit");
						HMISendText("b1","Start");
					}
				}
				else if (button == KANAN) {
					stepQuestion = 0;
					stepDelete = 0;
					
					if (f_eof(&fil) == 0){
						//HMISendText("t1","EOF 0");
						f_read(&fil,buffer,1000,&br);
						f_lseek(&fil,SPlast);
						f_puts(buffer,&fil);
						f_truncate(&fil);
						f_close(&fil);
					}
					else if (f_eof(&fil) == 1){
						//HMISendText("t1","EOF 1");
						f_lseek(&fil,SPlast);
						f_truncate(&fil);
						f_close(&fil);
					}
					
					HMISend("vis b0,1");
					HMISend("vis b1,1");
					HMISend("vis b2,0");
					HMISendText("t0","Press Start to Begin");
					HMISendText("t1","Question Cleared");
					HMISendText("b0","Edit");
					HMISendText("b1","Start");
				}
				
			}
		}
		else if (stepQuestion == STEPERROR){
				if (button == TENGAH) { 
					NVIC_SystemReset();
				}
		}
		
		
		/*
		if (stepQuestion == 0){
			if (button == TENGAH){
				button = 99;
				stepQuestion++;
			}
		}
		//================== ADD THE QUESTION FROM HERE ====================//
		else if (stepQuestion == 1)
			displayqABC(1,1,1,1,"Q=A+B*C","A=1,B=1,C=1");
		else if (stepQuestion == 2)
			displayqAB(0,0,0,"Q=A+B","A=0,B=0");
		else if (stepQuestion == 3)
			displayqABC(1,0,0,0,"Q=A'+B+C","A=1,B=0,C=0");
		else if (stepQuestion == 4)
			displayqABC(1,1,1,0,"Q=(A*B)*C'","A=1,B=1,C=1");
		else if (stepQuestion == 5)
			displayqAB(0,0,1,"Q=(A+B)'","A=0,B=0");
		//==================================================================//
		else if (stepQuestion == 6)
			displayScore();
		*/

		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OUT_TRUE_Pin|OUT_FALSE_Pin|OUT_BUZZ_Pin|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : IN_A_Pin IN_B_Pin IN_C_Pin IN_Q_Pin */
  GPIO_InitStruct.Pin = IN_A_Pin|IN_B_Pin|IN_C_Pin|IN_Q_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_TRUE_Pin OUT_FALSE_Pin OUT_BUZZ_Pin PB15 */
  GPIO_InitStruct.Pin = OUT_TRUE_Pin|OUT_FALSE_Pin|OUT_BUZZ_Pin|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
