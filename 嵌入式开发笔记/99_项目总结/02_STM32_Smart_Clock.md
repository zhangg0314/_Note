# 4x4矩阵键盘

## 1.原理介绍

### 1.硬件连接

![image-20250617202258823](..\\figure\image-20250617202258823.png)

- **行线与列线**
  在硬件上，4条行线连接到微控制器的输入引脚，而4条列线连接到微控制器的输出引脚。
- **按键位置**
  每个按键位于行线和列线的交叉点上。

### 2.`GPIO`初始化

1. **列**
   列线,通常设置为输出模式，用于向行线发送扫描信号。
2. **扫描信号**
   列线的主要功能是向行线发送扫描信号。当某一列线被设置为低电平时，与之相交的行线线会被检测（若相对应的按键被按下，行线被检测为低电平）以判断是否有按键被按下。
3. **确定按键列**
   通过依次将每一列线设置为低电平（可以逐列扫描键盘）。当检测到行线上有低电平时，可以确定被按下的按键位于当前选中的列上。若行线上无低电平，那么将此列线设置为高电平，下一列设置为低电平，进行新一轮按键检测。
4. **行**
   行线，设置为输入模式，用于读取按键状态。
5. **读取按键状态**
   行线的主要功能是读取按键状态。（`STM32`就靠行线来确认按键状态）当某一列线被选中（设置为低电平）时，微控制器会读取所有行线的电平状态，以判断是否有按键被按下。

### 3.注意事项

在实际应用中，还需要考虑去抖处理、中断处理等因素，以确保按键检测的准确性和稳定性。

## 2.实现方法

### 1.GPIO初始化

需要注意的是，GPIO引脚的主功能是上电时默认的功能，对于上电不是默认为IO配置的引脚需要进行重映射，这也对GPIO的操作才能生效，`GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);`

```c
void KeyBoard_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB ,ENABLE);//开启外设时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
    //行
    GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_InitStruct.GPIO_Pin = ROW1_PIN | ROW2_PIN | ROW3_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEYPAD_ROW_PORT,&GPIO_InitStruct);
    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_InitStruct.GPIO_Pin = ROW4_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEYPAD_COL_PORT,&GPIO_InitStruct);
	GPIO_StructInit(&GPIO_InitStruct);

    
    //列
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;//推挽输出
	GPIO_InitStruct.GPIO_Pin = COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEYPAD_COL_PORT,&GPIO_InitStruct);
    //初始所有列置高
    GPIO_SetBits(KEYPAD_COL_PORT, COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN)

#if 0
    //配置行引脚为中断模式
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource11);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource3);

    EXTI_InitTypeDef  EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line =  EXTI_Line3 | EXTI_Line11 | EXTI_Line12 | EXTI_Line15;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_Init(&NVIC_InitStructure);
#endif
}
```

### 2.列扫描

逐行逐列扫描法的原理：逐列将列线将依次置低电平，读取行线，如果某一条行线为低电平，则说明该行线与当前为低电平的列线交叉点处的按键被按下，从判断按键按下。

- 伪代码

- ```c
  初始化GPIO;
  int i = 0;
  int col = -1;
  int row = -1;
  int debonuceflag = 0;
  while(1)
  {
      拉低第(i)列，
      row = 扫描行，
      if(row ！= -1 && !debonuceflag)
      {	
          Gettimer();
          col = i;
          debonuceflag = 1;
      }
      if(debonuceflag && timer > 20ms)
      {
          debonuceflag = 0;
        	relesetimer();
          拉低第（col）列；t
          电平 = 读取第（row）电平;
          if(电平 == 0)
          {
              Printf("%d按键按下了\n",Map[row][col]);
              delay(200ms);
          }
          col = -1;
          row = -1;
      }
  }
  ```

- 项目代码

- ```c
  volatile uint8_t g_currentCol = 0;
  volatile uint8_t row, col = 0;
  int debonuceflag = 0;
  
  uint8_t uiKEYBPARD_Process(void)
  {
      uint8_t key = 0;
      KEYBOARD_ScanColumn(g_currentCol);
      int i;
      for (i = 0; i < 4; i++)
      {
          if (KEYBOARD_GetRowStatus(i) == 0 && !debonuceflag)
          {
              Get_Timer();
              row = i;
              col = g_currentCol;
              debonuceflag = 1;
          }
      }
      if (debonuceflag && Timer_Count > 20)
      {
          debonuceflag = 0;
          Release_Timer();
          KEYBOARD_ScanColumn(col);
          if (KEYBOARD_GetRowStatus(row) == 0)
          {
              LOG_DEBUG("%c\r\n", KEY_MAP[row][col]);
              key = KEY_MAP[row][col];
              Delayus(200 * 1000);//延时一下，防抖
          }
          col = 0;
          row = 0;
      }
      g_currentCol = (g_currentCol + 1) % 4;
      return key;
  }
  ```

- ### 3.中断模式（非中断里延时）

- 伪代码

  ```c
  volatile uint8_t g_currentCol = 0;
  volatile uint8_t row, col = 0;
  int debonuceflag = 0;
  
  void 中断处理函数(void)//假设为第row行的中断处理函数
  {
       if(EXTI_GetITStatus(EXTI_Line3) == SET)
  	{
          清除中断标志位;
          if(未开启防抖)
          {
             开启防抖；
             获取定时器;
             col = g_currentCol;
          }
          else (开启防抖)
          {
              if(Timer_Count > 500ms)
              {
                  Release_Timer();
                  拉低第(col)列；
                  关闭防抖；
                  if(KEYBOARD_GetRowStatus(row) == 0)
                  {
                      keyvalue = Map[row][col];
                  }
                  row = 0;
                  col = 0;
              }
          }
       }
  }
  ```

- 项目代码

  ```c
  // 扫描指定列
  void KEYBOARD_ScanColumn(uint8_t col) {
      // 根据列号设置对应列为低电平
      // 所有列设为高电平
      GPIO_SetBits(KEYPAD_COL_PORT, COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN);
      
      switch(col) {
          case 0: GPIO_ResetBits(KEYPAD_COL_PORT, COL1_PIN); break;
          case 1: GPIO_ResetBits(KEYPAD_COL_PORT, COL2_PIN); break;
          case 2: GPIO_ResetBits(KEYPAD_COL_PORT, COL3_PIN); break;
          case 3: GPIO_ResetBits(KEYPAD_COL_PORT, COL4_PIN); break;
      }
  }
  
  // 检测行状态
  static uint8_t KEYBOARD_GetRowStatus(uint8_t row) {
      switch(row) {
          case 0: return GPIO_ReadInputDataBit(KEYPAD_ROW_PORT, ROW1_PIN);
          case 1: return GPIO_ReadInputDataBit(KEYPAD_ROW_PORT, ROW2_PIN);
          case 2: return GPIO_ReadInputDataBit(KEYPAD_ROW_PORT, ROW3_PIN);
          case 3: return GPIO_ReadInputDataBit(KEYPAD_COL_PORT, ROW4_PIN);
          default: return 1;
      }
  }
  static void KEYBPARD_Process(uint8_t row)
  {
      if (!debonuceflag)
      {
          debonuceflag = 1;
          Get_Timer();
          col = g_currentCol;
      }
      else
      {
          if (Timer_Count > 20)
          {
              KEYBOARD_ScanColumn(col);
              debonuceflag = 0;
              Release_Timer();
              if (KEYBOARD_GetRowStatus(row) == 0)
              {
                  g_keyvalue = KEY_MAP[row][col];
              }
              else
              {
                  g_keyvalue = 0;
              }
              row = 0;
              col = 0;
          }
      }
  }
  void EXTI3_IRQHandler(void)
  {
  
      if (EXTI_GetITStatus(EXTI_Line3) == SET)
      {
          EXTI_ClearITPendingBit(EXTI_Line3);
          KEYBPARD_Process(3);
         
      }
  }
  void EXTI15_10_IRQHandler(void)
  {
      if (EXTI_GetITStatus(EXTI_Line11) == SET)
      {
          EXTI_ClearITPendingBit(EXTI_Line11);
          KEYBPARD_Process(0);
      }
      else if (EXTI_GetITStatus(EXTI_Line12) == SET)
      {
          EXTI_ClearITPendingBit(EXTI_Line12);
          KEYBPARD_Process(1);
      }
      else if (EXTI_GetITStatus(EXTI_Line15) == SET)
      {
  
          EXTI_ClearITPendingBit(EXTI_Line15);
          KEYBPARD_Process(2);
      }
      else
      {
      }
  }
  
  ===============================================================================================
        case STATE_PASSWORD_INPUT:
  				if(g_lastState != g_currentState)
  				{
  					//静态页面展示，且大小长度固定的字符串
  					g_lastState = g_currentState;
  					LCD_Clear();
  					LCD_SetCursor(0,0);
  					LCD_Write_String("Enter Password:");
  					LCD_SetCursor(0,1);             
  				}
  				KEYBOARD_ScanColumn(g_currentCol);
  				if(g_keyvalue != 0)
  				{
  					LCD_Write_Char(g_keyvalue);
  					Delayus(200*1000);//延时一下，防抖
  					g_keyvalue = 0;
  				}
  				g_currentCol = (g_currentCol + 1) % 4;
  				
  			}
  ```

  







