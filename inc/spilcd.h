#ifndef SPILCD_H_
#define SPILCD_H_

#include "main.h"
#include <cmath>
#include "string.h"
#include "stdio.h"

class Spi1_interface
{
public:
    //void reset_active(){GPIOA->ODR&=~GPIO_ODR_ODR_2;}
    //void reset_idle(){GPIOA->ODR|=GPIO_ODR_ODR_2;}
    //void cs_active(){GPIOA->ODR&=~GPIO_ODR_ODR_4;}
    //void cs_idle(){GPIOA->ODR|=GPIO_ODR_ODR_4;}
    //void dc_command(){GPIOA->ODR&=~GPIO_ODR_ODR_3;}
    //void dc_data(){GPIOA->ODR|=GPIO_ODR_ODR_3;}
    
    inline void reset_active(){GPIOC->ODR&=~GPIO_ODR_ODR_13;}
    inline void reset_idle(){GPIOC->ODR|=GPIO_ODR_ODR_13;}
    inline void dc_command(){GPIOC->ODR&=~GPIO_ODR_ODR_14;}
    inline void dc_data(){GPIOC->ODR|=GPIO_ODR_ODR_14;}
    inline void cs_active(){GPIOC->ODR&=~GPIO_ODR_ODR_15;}
    inline void cs_idle(){GPIOC->ODR|=GPIO_ODR_ODR_15;}
    

    void tft_reset() //reset TFT дергаем ножкой reset
    {
        reset_active();   
        delay(168000*1);     
        reset_idle();
        tft_SendCommand(0x01);
        delay(168000*1);
    }    
    void tft_SendCommand(uint8_t cmd)
    {
        while((SPI1->SR&SPI_SR_BSY)){}
        dc_command();
        spi_sendByte(cmd);  
        delay(40);
    }
    void tft_sendData(uint8_t data)
    {        
        dc_data();
        spi_sendByte(data);
    }    
    void spi_sendByte(uint8_t data)   // the most important function
    {
        while(!(SPI1->SR&SPI_SR_TXE)){}        
        SPI1->DR=data;
        //*(uint8_t*)SPI1->DR=data;
        while((SPI1->SR&SPI_SR_BSY)){}
    }
    void spi_sendWord(uint16_t data)
    {
        //ждем пока освободится буффер передатчика
        while((SPI1->SR&SPI_SR_BSY)){}
        dc_data();
        tft_sendData(data>>8);
        while((SPI1->SR&SPI_SR_BSY)){}
        tft_sendData(data&0x00FF);
        //while((SPI1->SR&SPI_SR_BSY)){}
    }
    void tft_setColumn(int StartCol, int EndCol)
    {
        tft_SendCommand(0x2A);                                                     // Column Command address
        spi_sendWord(StartCol);
        spi_sendWord(EndCol);
    }
    void tft_setRow(int StartRow, int EndRow)
    {
        tft_SendCommand(0x2B);                                                  // Column Command address
        spi_sendWord(StartRow);
        spi_sendWord(EndRow);
    }
    void tft_setXY(int poX, int poY)
    {
        tft_setColumn(poX, poX);
        tft_setRow(poY, poY);
        tft_SendCommand(0x2C);
    }
    void tft_setPixel(int poX, int poY, int color)
    {
        tft_setXY(poX, poY);
        spi_sendWord(color);
        spi_sendWord(color);
    }
    void swap(uint16_t x1, uint16_t x2)
    {
        if(x2>x1){uint16_t z=x2;x2=x1;x1=z;}
    }
    void checkXYswap(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
    {
        swap(x1,x2); swap(y1,y2);
    }

    uint16_t spi_read()
    {
        SPI1->DR = 0; //запускаем обмен
        while(!(SPI1->SR&SPI_SR_RXNE)) //ждем пока не появится новое значение в буффере приемника
        return SPI1->DR; //возвращаем значение из буффера приемника
    }
    void fillScreen(uint16_t color)
    {
        tft_setRow(0, 240);
        tft_setColumn(0,320);        
        tft_SendCommand(0x2C);
        //dc_data();
        //cs_active();
        for(volatile uint32_t i=0; i<76800; i++)
        {           
            //while((SPI1->SR&SPI_SR_BSY)){}
            spi_sendWord(color);           
        }
    }        
    void delay(volatile uint32_t x)
    {
        for(volatile uint32_t i=0;i<x;i++){;}
    }
};

class SpiLcd:public Spi1_interface
{
public:
    SpiLcd(){spi1_ini(); tft_ini(320,240);fillScreen(0x0000);}

private:
    void spi1_ini()
    {
        //------------- настройка RESET (display reset)  PC-13-------------------
        RCC->AHB1ENR|=RCC_AHB1ENR_GPIOCEN;
        GPIOC->MODER|=GPIO_MODER_MODER13_0;        
        GPIOC->MODER&=~GPIO_MODER_MODER13_1;
        GPIOC->OTYPER=0;//
        GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR13;//
        GPIOC->ODR=0;//
        
        //RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
        //GPIOA->MODER|=GPIO_MODER_MODER2_0;
	    //GPIOA->MODER&=~GPIO_MODER_MODER2_1; // 01: General purpose output mode
	    //GPIOA->OTYPER=0; //0: Output push-pull (reset state)
	    //GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR2; //11: High speed
        //GPIOA->ODR=0; // low level

        //------------- настройка DC (data command) PC-14-------------------
        GPIOC->MODER|=GPIO_MODER_MODER14_0;        
        GPIOC->MODER&=~GPIO_MODER_MODER14_1;        
        GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR14;//
        
        //RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
        //GPIOA->MODER|=GPIO_MODER_MODER3_0;
	    //GPIOA->MODER&=~GPIO_MODER_MODER3_1; // 01: General purpose output mode
	    //GPIOA->OTYPER=0; //0: Output push-pull (reset state)        
	    //GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR3; //11: High speed
	    
        //------------- настройка CS (chip select NSS) PC-15-------------------
        GPIOC->MODER|=GPIO_MODER_MODER15_0;//
        GPIOC->MODER&=~GPIO_MODER_MODER15_1;//
        GPIOC->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR15;//
        //RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;
        //GPIOA->MODER|=GPIO_MODER_MODER4_0;
	    //GPIOA->MODER&=~GPIO_MODER_MODER4_1; // 01: General purpose output mode
	    //GPIOA->OTYPER=0; //0: Output push-pull (reset state)        
	    //GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR4; //11: High speed
	    //GPIOA->ODR=0; // low level

        //------------- настройка ножек SPI:  PA5-_SCK, PA6-MISO, PA7-MOSI :::AF5 (PB3-SCK, PB4-MISO, PB5-MOSI)
        
        RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN;
        GPIOB->MODER|=GPIO_MODER_MODER3_1;//
        GPIOB->MODER&=~GPIO_MODER_MODER3_0;//
        GPIOB->MODER|=GPIO_MODER_MODER5_1;//
        GPIOB->MODER&=~GPIO_MODER_MODER5_0;//
        GPIOB->AFR[0]=0;
        GPIOB->AFR[0]|=(5<<12)|(5<<20);//
        GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR3_0|GPIO_PUPDR_PUPDR3_1);        
        GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR5_0|GPIO_PUPDR_PUPDR5_1);
        GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR3;//
        GPIOB->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR5;//
        
        //GPIOA->MODER|=GPIO_MODER_MODER5_1;
        //GPIOA->MODER&=~GPIO_MODER_MODER5_0;//10: PA-5 alternate func
        //GPIOA->MODER|=GPIO_MODER_MODER7_1;
        //GPIOA->MODER&=~GPIO_MODER_MODER7_0;//10: PA-7 alternate func
        //GPIOA->AFR[0]|=(5<<20)|(5<<28);//SCK-PA5 MOSI-PA7   
        ////Отключение подтяжки на PA5, PA7
        //GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR5_0|GPIO_PUPDR_PUPDR5_1);        
        //GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR7_0|GPIO_PUPDR_PUPDR7_1);           
	    //GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR5;
        //GPIOA->OSPEEDR|=GPIO_OSPEEDER_OSPEEDR7; //11: high speed

        //------------- тактируем SPI-1  ---------------------------
        RCC->APB2ENR|=RCC_APB2ENR_SPI1EN; //clock on fast SPI-1
        //------------- предделитель SPI-1 -----------------------------
        SPI1->CR1&=~SPI_CR1_BR; // (000=>psc=1) 1:1:1  => 84000000/256 = 328.125 kHz - SPI-1 clk
        //------------- настройка SPI - 1 -----------------------------------------
        SPI1->CR1&=~SPI_CR1_BIDIMODE; // двухпроводной режим работы
        SPI1->CR1|=SPI_CR1_BIDIOE; //1: Output enabled (transmit-only mode)
        SPI1->CR1&=~SPI_CR1_CRCEN; // 0: CRC calculation disabled
        SPI1->CR1&=~SPI_CR1_CRCNEXT; //0: Data phase (no CRC phase)
        SPI1->CR1&=~SPI_CR1_DFF; //0: 8-bit data frame format is selected for transmission/reception
        SPI1->CR1&=~SPI_CR1_RXONLY; //0: Full duplex (Transmit and receive)
        SPI1->CR1|=SPI_CR1_SSM; // программное управление ведомым включено 
        SPI1->CR1|=SPI_CR1_SSI; // нужно чтобы эти два бита были в 1 для правильной инициализации мастера
        SPI1->CR1&=~SPI_CR1_LSBFIRST; //1: LSB first //0: MSB transmitted first
        SPI1->CR1|=SPI_CR1_MSTR; //1: Master configuration
        SPI1->CR1&=~SPI_CR1_CPOL; //1: CK to 1 when idle (смотреть в datasheet slave) 
        SPI1->CR1&=~SPI_CR1_CPHA; //1: The second clock transition is the first data capture edge (тоже)
        
        //NVIC_EnableIRQ(SPI1_IRQn);
        //----------- включаем SPI-1 --------------------------------------------
        SPI1->CR1|=SPI_CR1_SPE;
    }
     void tft_ini(uint16_t w_size, uint16_t h_size)
    {  
        //cs_idle();
        dc_data();
        //cs_active();
        tft_reset(); 
        //delay(10000);
        //tft_SendCommand(0x28); //display off
        //---------------------------------------------------
        tft_SendCommand(0xEF);
        tft_sendData(0x03);
        tft_sendData(0x80);
        tft_sendData(0x02);
        //Power Control A
        tft_SendCommand(0xCB);  
        tft_sendData(0x39);
        tft_sendData(0x2C);
        tft_sendData(0x00);
        tft_sendData(0x34);
        tft_sendData(0x02); 
        //Power Control B
        tft_SendCommand(0xCF);
        tft_sendData(0x00);
        tft_sendData(0xC1);
        tft_sendData(0x30);
        //Power on Sequence control
        tft_SendCommand(0xED);
        tft_sendData(0x64);
        tft_sendData(0x03);
        tft_sendData(0x12);
        tft_sendData(0x81);        
        //Driver timing control A
        tft_SendCommand(0xE8);
        tft_sendData(0x85);
        tft_sendData(0x00);
        tft_sendData(0x78);        
        //Driver timing control B
        tft_SendCommand(0xEA);
        tft_sendData(0x00);
        tft_sendData(0x00);                
        //Pump ratio control
        tft_SendCommand(0xF7);
        tft_sendData(0x20);
        //Power Control,VRH[5:0]
        tft_SendCommand(0xC0);//Power contro
        tft_sendData(0x23);  //VRH[5:0
        //Power Control,SAP[2:0];BT[3:0]
        tft_SendCommand(0xC1);
        tft_sendData(0x10);   //SAP[2:0];BT[3:0
        //VCOM Control 1
        tft_SendCommand(0xC5);//VCM control
        tft_sendData(0x3E);  //Contrast
        tft_sendData(0x28);
        //VCOM Control 2
        tft_SendCommand(0xC7);//VCM control2
        tft_sendData(0x86);
        tft_SendCommand(0x37);
        tft_sendData(0x00);
        //Memory Acsess Control - rotation
        tft_SendCommand(0x36);
        tft_sendData(0xf8);  // 1-полубайт ориентация (через 2) - 2-ой цветовая схема (0 или 8)
        //Pixel Format Set
        tft_SendCommand(0x3A);
        tft_sendData(0x55);//16bit
        //Frame Rratio Control, Standard RGB Color
        tft_SendCommand(0xB1);
        tft_sendData(0x00);
        tft_sendData(0x18);
        tft_SendCommand(0xB6);      // Display Function Control
        tft_sendData(0x08); 
        tft_sendData(0x82); 
        tft_sendData(0x27);
        //Enable 3G (пока не знаю что это за режим)
        tft_SendCommand(0xF2);
        tft_sendData(0x00);//не включаем        
        //Gamma set
        tft_SendCommand(0x26);
        tft_sendData(0x01);//Gamma Curve (G2.2) (Кривая цветовой гаммы)
        //Positive Gamma  Correction
        tft_SendCommand(0xE0);
        tft_sendData(0x0F);
        tft_sendData(0x31);
        tft_sendData(0x2B);
        tft_sendData(0x0C);
        tft_sendData(0x0E);
        tft_sendData(0x08);
        tft_sendData(0x4E);
        tft_sendData(0xF1);
        tft_sendData(0x37);
        tft_sendData(0x07);
        tft_sendData(0x10);
        tft_sendData(0x03);
        tft_sendData(0x0E);
        tft_sendData(0x09);
        tft_sendData(0x00);
        //Negative Gamma  Correction
        tft_SendCommand(0xE1);
        tft_sendData(0x00);
        tft_sendData(0x0E);
        tft_sendData(0x14);
        tft_sendData(0x03);
        tft_sendData(0x11);
        tft_sendData(0x07);
        tft_sendData(0x31);
        tft_sendData(0xC1);
        tft_sendData(0x48);
        tft_sendData(0x08);
        tft_sendData(0x0F);
        tft_sendData(0x0C);
        tft_sendData(0x31);
        tft_sendData(0x36);
        tft_sendData(0x0F);

        tft_SendCommand(0x2B); //page set
        tft_sendData(0x00);
        tft_sendData(0x00);
        tft_sendData(0x00);
        tft_sendData(0xEF);
        
        tft_SendCommand(0x2A); // column set
        tft_sendData(0x00);
        tft_sendData(0x00);
        tft_sendData(0x01);
        tft_sendData(0x3F);
        
        tft_SendCommand(0x34); //tearing effect off
        tft_SendCommand(0xB7); //entry mode set
        tft_sendData(0x07);
        
        tft_SendCommand(0x11);//Выйдем из спящего режима
        delay(1680000); //100 ms
        //Display ON
        
        tft_SendCommand(0x29);//display on
        //tft_sendData(TFT9341_ROTATION);        
        
        delay(1680000); //100 ms       
        tft_SendCommand(0x13);        
    }
};
// ///////////////////////////////////////////////////////////////////////////
//*********** Класс обработки цифр и букв ********************
class Font_interface:public Spi1_interface
{
public:
    Font_interface(){}
    uint32_t char_to_int(char* str,uint8_t size)
   {
       uint32_t x;
       for(uint8_t i=0;i<size;i++)
       {
           uint8_t dec;
           if (str[i]==48){dec=0;} if (str[i]==49){dec=1;}
           if (str[i]==50){dec=2;} if (str[i]==51){dec=3;}
           if (str[i]==52){dec=4;} if (str[i]==53){dec=5;}
           if (str[i]==54){dec=6;} if (str[i]==55){dec=7;}
           if (str[i]==56){dec=8;} if (str[i]==57){dec=9;}
           x+=dec*pow(10,size-i);           
       }
       return x;
   }
    char arr[20];
    char arrFloat[20]{0};
    volatile uint8_t arrSize=0;

    void intToChar(uint32_t x) //int to char*
    {
        uint32_t y=x;        
        uint8_t count=0;
        while (y>0)
        {  y=y/10;  count++; }//считаем количество цифр
        y=x;
        
        arrSize=count;
        if(x==0) {arrSize=1;arr[arrSize-1]='0';arr[arrSize]='\0';return;} 
        for(uint8_t i=0;i<arrSize;i++)
        {            
            int x=y%10;
            if(x==0) {arr[arrSize-1-i]='0';} if(x==1) {arr[arrSize-1-i]='1';}
            if(x==2) {arr[arrSize-1-i]='2';} if(x==3) {arr[arrSize-1-i]='3';}
            if(x==4) {arr[arrSize-1-i]='4';} if(x==5) {arr[arrSize-1-i]='5';}
            if(x==6) {arr[arrSize-1-i]='6';} if(x==7) {arr[arrSize-1-i]='7';}
            if(x==8) {arr[arrSize-1-i]='8';} if(x==9) {arr[arrSize-1-i]='9';}
            y=y/10;        
        }
        if(arrSize+1<10)
        {
            //strcat(arr+arrSize,'\0'); 
            arr[arrSize]='\0';
        }
    }
    void floatTochar(float x)
    {        
        sprintf(arrFloat, "%.3f", x);           
    }
};
class Font_8x8: public Font_interface
{
public:
    Font_8x8()
    {}
    void symbol(uint16_t x, uint16_t y, uint16_t col,const uint8_t* data)
    {
        tft_setColumn(x,x+8);
        tft_setRow(y,y+8);
        tft_SendCommand(0x2C);
        for(uint16_t i=0; i <y+8-y;i++)   //заполняем 8-битную матрицу
        {
            //spi_sendWord(col);
            for(uint16_t j=0; j <x+8-x+1;j++)
            {
                if((j==0)&&(data[i]&0x80)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==1)&&(data[i]&0x40)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==2)&&(data[i]&0x20)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==3)&&(data[i]&0x10)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==4)&&(data[i]&0x08)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==5)&&(data[i]&0x04)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==6)&&(data[i]&0x02)) {spi_sendWord(col);}//else spi_sendWord(0xF800);
                else if((j==7)&&(data[i]&0x01)) {spi_sendWord(col);}
                else spi_sendWord(0x0000);
            }
        }
    }    
    void print(uint16_t x, uint16_t y,uint16_t col,char* str)
    {
        for(uint8_t i=0;i<strlen(str);i++)
        {
            if (str[i]==48){symbol(x, y, col, data0);}
            if (str[i]==49){symbol(x, y, col, data1);}
            if (str[i]==50){symbol(x, y, col, data2);}
            if (str[i]==51){symbol(x, y, col, data3);}
            if (str[i]==52){symbol(x, y, col, data4);}
            if (str[i]==53){symbol(x, y, col, data5);}
            if (str[i]==54){symbol(x, y, col, data6);}
            if (str[i]==55){symbol(x, y, col, data7);}
            if (str[i]==56){symbol(x, y, col, data8);}
            if (str[i]==57){symbol(x, y, col, data9);}
            x+=9;            
        }
    }    
    const uint8_t data0[8]={0x7C,0xFE,0xC6,0xC6,0xC6,0xC6,0xFE,0x7C}; //0         
    const uint8_t data1[8]={0x30,0x70,0xF0,0x30,0x30,0x30,0x30,0x30}; //1         
    const uint8_t data2[8]={0x78,0xFC,0xCC,0x1C,0x38,0x70,0xFC,0xFC}; //2         
    const uint8_t data3[8]={0x78,0xFC,0x0C,0x78,0xFC,0x0C,0xFC,0x78}; //3         
    const uint8_t data4[8]={0x1C,0x3C,0x6C,0xCC,0xFC,0x0C,0x0C,0x0C}; //4         
    const uint8_t data5[8]={0xFC,0xC0,0xC0,0xF8,0x0C,0xCC,0xFC,0x78}; //5         
    const uint8_t data6[8]={0x7E,0xC6,0xC0,0xFC,0xC6,0xC6,0xFE,0x7C}; //6         
    const uint8_t data7[8]={0xFE,0xFE,0x06,0x0E,0x1C,0x38,0x70,0x60}; //7         
    const uint8_t data8[8]={0x7C,0xFE,0xC6,0x7C,0x7C,0xC6,0xFE,0x7C}; //8         
    const uint8_t data9[8]={0x7C,0xFE,0xC6,0x7E,0x06,0x86,0xFE,0x7C}; //9 
};

class Font_16x16: public Font_interface
{
public:
    Font_16x16(){DMA_ini();}
    void DMA_ini()
    {
        RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN; // clock on DMA2
        DMA2_Stream3->CR|=DMA_SxCR_CHSEL_0|DMA_SxCR_CHSEL_1; //0:1:1 -3 channel SPI1 Tx
        DMA2_Stream3->CR|=DMA_SxCR_DIR_0; // 0:1 from memory to peripheral
        DMA2_Stream3->NDTR = 65535;

        DMA2_Stream3->CR|=DMA_SxCR_MSIZE_0; //memory size data 16
        DMA2_Stream3->CR|=DMA_SxCR_PSIZE_0; //peripheral size data 16    (8)
        DMA2_Stream3->CR|=DMA_SxCR_MINC; //memory address incrementation after transmit
        DMA2_Stream3->CR&=~DMA_SxCR_PINC; //peripheral address not incrementated after transmit

        DMA2_Stream3->PAR=(uint32_t)&SPI1->DR;
        //DMA2_Stream3->M0AR = (uint32_t)&image_data_My;
        //DMA2_Stream3->CR|=DMA_SxCR_TCIE; // transfer complete interrupt enabled

        SPI1->CR2|=SPI_CR2_TXDMAEN;
        
    }
    void printPhoto(const uint16_t* data)
    {
        tft_setRow(0,240);
        tft_setColumn(0,320);       
        tft_SendCommand(0x2C);
        dc_data();
        //DMA2_Stream3->CR&=~DMA_SxCR_EN;//disable DMA for new setup
        SPI1->CR1|=SPI_CR1_DFF; //16 bits
        DMA2_Stream3->CR|=DMA_SxCR_EN; // start again 
        //for(uint32_t i=0;i<76800;i++)
        //{
        //    spi_sendWord(*(data+i));
        //    //spi_sendWord(0x00ff);
        //}
    }
    void forPrint()
    {
        DMA2_Stream3->CR&=~DMA_SxCR_EN;//disable DMA for new setup        
                
        
        //while(!(DMA2->LISR&DMA_LISR_TCIF3)){}
        DMA2->LIFCR|=DMA_LIFCR_CTCIF3; //clear status flag about transfer complete        
        //DMA2_Stream3->M0AR = (uint32_t)(&image_data_My)+65535*2;
        DMA2_Stream3->NDTR = 11265;
        DMA2_Stream3->CR|=DMA_SxCR_EN; // start again      
        //SPI1->CR1&=~SPI_CR1_DFF; // again 8 bits
    }
    
    void symbol(uint16_t x, uint16_t y, uint16_t col,const uint16_t* data)
    {        
        tft_setColumn(x,x+16);
        tft_setRow(y,y+16);
        tft_SendCommand(0x2C);
        for(uint16_t i=0; i <y+16-y;i++)   //заполняем 8-битную матрицу
        {
            //spi_sendWord(col);
            for(uint16_t j=0; j <x+16-x+1;j++)
            {
                if((j==0)&&(data[i]&0x8000)) {spi_sendWord(col);}
                else if((j==1)&&(data[i]&0x4000)) {spi_sendWord(col);}
                else if((j==2)&&(data[i]&0x2000)) {spi_sendWord(col);}
                else if((j==3)&&(data[i]&0x1000)) {spi_sendWord(col);}
                else if((j==4)&&(data[i]&0x0800)) {spi_sendWord(col);}
                else if((j==5)&&(data[i]&0x0400)) {spi_sendWord(col);}
                else if((j==6)&&(data[i]&0x0200)) {spi_sendWord(col);}
                else if((j==7)&&(data[i]&0x0100)) {spi_sendWord(col);}
                else if((j==8)&&(data[i]&0x0080)) {spi_sendWord(col);}
                else if((j==9)&&(data[i]&0x0040)) {spi_sendWord(col);}
                else if((j==10)&&(data[i]&0x0020)) {spi_sendWord(col);}
                else if((j==11)&&(data[i]&0x0010)) {spi_sendWord(col);}
                else if((j==12)&&(data[i]&0x0008)) {spi_sendWord(col);}
                else if((j==13)&&(data[i]&0x0004)) {spi_sendWord(col);}
                else if((j==14)&&(data[i]&0x0002)) {spi_sendWord(col);}
                else if((j==15)&&(data[i]&0x0001)) {spi_sendWord(col);}
                else spi_sendWord(0x0000);
            }
        }       
    }    
    void clearString(uint16_t x, uint16_t y,uint8_t size)
    {
        tft_setColumn(x,x+16*size);
        tft_setRow(y,y+16);
        tft_SendCommand(0x2C);
        for(uint16_t i=0; i <(y+16+1)*(x+16+1);i++)   //заполняем 8-битную матрицу
        {
            spi_sendWord(0x0000);
        }
    }
    void print(uint16_t x, uint16_t y,uint16_t col,const char* str,uint8_t numDigits)
    {        
        for(uint8_t i=0;i<strlen(str);i++)
        {
            if (str[i]==48){symbol(x, y, col, data0);}
            if (str[i]==49){symbol(x, y, col, data1);}
            if (str[i]==50){symbol(x, y, col, data2);}
            if (str[i]==51){symbol(x, y, col, data3);}
            if (str[i]==52){symbol(x, y, col, data4);}
            if (str[i]==53){symbol(x, y, col, data5);}
            if (str[i]==54){symbol(x, y, col, data6);}
            if (str[i]==55){symbol(x, y, col, data7);}
            if (str[i]==56){symbol(x, y, col, data8);}
            if (str[i]==57){symbol(x, y, col, data9);}
            if (str[i]==58){symbol(x, y, col, dataColon);}
            if (str[i]==46){symbol(x, y, col, dataPoint);x+=17;}
            else{x+=17;}                        
        }
        for(uint8_t i=strlen(str);i<strlen(str)+numDigits-1;i++)
        {
            {symbol(x, y, col, dataSpace);}
            x+=17;            
        }
    }    
    const uint16_t data0[16]={0x0FE0,0x1FF0,0x3FF8,0x7C7C,0x783C,0xF01E,0xF01E,0xF01E,          
                              0xF01E,0xF01E,0x783C,0x7C7C,0x3FF8,0x1FF0,0x0FE0,0x0000}; //0         
    const uint16_t data1[16]={0x07F8,0x0FF8,0x1FF8,0x3FF8,0x7FF8,0xFFF8,0xFDF8,0xF9F8,          
                              0x01F8,0x01F8,0x01F8,0x01F8,0x01F8,0x01F8,0x01F8,0x0000}; //1         
    const uint16_t data2[16]={0x3FF0,0x7FF8,0xFFFC,0xF0FC,0xF0FC,0xE1F8,0x03F0,0x07E0,         
                              0x0FC0,0x1F80,0x3F00,0x7E00,0xFFFC,0xFFFC,0xFFFC,0x0000}; //2         
    const uint16_t data3[16]={0x3FF0,0x7FF8,0xFFFC,0xE07C,0x003C,0x007C,0x3FF8,0x7FF0,          
                              0x3FF8,0x007C,0x003C,0xE07C,0xFFFC,0x7FF8,0x3FF0,0x0000}; //3         
    const uint16_t data4[16]={0x03FC,0x07FC,0x0FFC,0x1E7C,0x3C7C,0x787C,0xF07C,0xFFFC,          
                              0xFFFC,0xFFFC,0x007C,0x007C,0x007C,0x007C,0x007C,0x0000}; //4         
    const uint16_t data5[16]={0xFFFC,0xFFFC,0xFFFC,0xE000,0xE000,0xFFE0,0xFFF0,0xFFF8,          
                              0x007C,0x003C,0xE03C,0xF07C,0xFFF8,0x7FF0,0x3FE0,0x0000}; //5         
    const uint16_t data6[16]={0x3FF8,0x7FFC,0xFFFC,0xF03C,0xE000,0xF000,0xFFF0,0xFFF8,          
                              0xFFFC,0xF03C,0xE01C,0xF03C,0xFFF8,0x7FF0,0x3FE0,0x0000}; //6         
    const uint16_t data7[16]={0x7FFC,0xFFFC,0xFFFC,0xFFFC,0x003C,0x007C,0x00FC,0x01F8,          
                              0x03F0,0x07E0,0x0FC0,0x1F80,0x3F00,0x7E00,0xFC00,0x0000}; //7         
    const uint16_t data8[16]={0x1FE0,0x3FF0,0x7FF8,0xF87C,0xF03C,0xF87C,0x7FF8,0x3FF0,          
                              0x7FF8,0xF87C,0xF03C,0xF87C,0x7FF8,0x3FF0,0x1FE0,0x0000}; //8         
    const uint16_t data9[16]={0x1FE0,0x3FF0,0x7FF8,0xF03C,0xF03C,0xF03C,0xFFFC,0x7FFC,          
                              0x3FFC,0x003C,0xE07C,0xF0F8,0xFFF0,0x7FE0,0x3FC0,0x0000}; //9
    const uint16_t dataColon[16]={0x0000,0x0000,0x0780,0x0780,0x0780,0x0780,0x0000,0x0000,          
                              0x0000,0x0780,0x0780,0x0780,0x0780,0x0000,0x0000,0x0000};  //:  
    const uint16_t dataSpace[16]={0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,          
                              0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};  //Space  
    const uint16_t dataPoint[16]={0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,          
                              0x0000,0x0000,0x0000,0x03C0,0x03C0,0x03C0,0x03C0,0x0000};  //.  
};
class Time:Font_16x16
{
public:
    Time(){}
    static volatile bool colon;
    void printTime(uint16_t x, uint16_t y,uint16_t col,uint8_t hour, uint8_t min, uint8_t sec)
    {
        if(Time::colon){Time::colon=false;}
        else {Time::colon=true;}
        if(hour<10)    // печатаем часы
        {
            intToChar(hour);
            print(x,y,col,"0",1);
            print(x+17,y,col,arr,1);
        }
        else
        {
            intToChar(hour);
            print(x,y,col,arr,2);
        }
        if(colon){print(x+17*2,y,col,":",1);}
        else{symbol(x+17*2,y,col,dataSpace);}
        

        if(min<10)      // печатаем минуты      
        {
            intToChar(min);
            print(x+17*3,y,col,"0",1);
            print(x+17*4,y,col,arr,1);
        }
        else
        {
            intToChar(min);
            print(x+17*3,y,col,arr,2);
        }
        if(colon){print(x+17*5,y,col,":",1);}
        else{symbol(x+17*5,y,col,dataSpace);}
        

        if(sec<10)       //печатаем секунды
        {
            intToChar(sec);
            print(x+17*6,y,col,"0",1);
            print(x+17*7,y,col,arr,1);
        }
        else
        {
            intToChar(sec);
            print(x+17*6,y,col,arr,2);
        }
    }
};
volatile bool Time::colon=0;

class Pixel:public Spi1_interface
{
public:
    Pixel(uint16_t x=0,uint16_t y=0,uint16_t col=BLACK)
    {
        //setPixel(x,y,col);  
    }
    enum col
    {
        RED        =    0xf800,
        GREEN      =    0x07e0,
        BLUE       =    0x001f,
        BLACK      =    0x0000,
        YELLOW     =    0xffe0,
        WHITE      =    0xffff,
        CYAN       =    0x07ff,
        BRIGHT_RED =    0xf810,
        GRAY1      =    0x8410,
        GRAY2      =    0x4208
    };

    void setPixel(uint16_t X, uint16_t Y, uint16_t color)
    {
        tft_setXY(X, Y);
        spi_sendWord(color);
        //spi_sendWord(color);
    }
};
class Rect:public Spi1_interface//, public Pixel
{
public:
    enum col
    {
        RED        =    0xf800,
        GREEN      =    0x07e0,
        BLUE       =    0x001f,
        BLACK      =    0x0000,
        YELLOW     =    0xffe0,
        WHITE      =    0xffff,
        CYAN       =    0x07ff,
        BRIGHT_RED =    0xf810,
        GRAY1      =    0x8410,
        GRAY2      =    0x4208
    };
public: 
    Rect(uint16_t x1=0,uint16_t x2=0,uint16_t y1=0,uint16_t y2=0,uint16_t col=BLACK)
    {        
    }   
    void setRect(uint16_t x1, uint16_t y1, uint16_t x2,uint16_t y2,uint16_t col) 
    {
        checkXYswap(x1,y1,x2,y2);
        tft_setColumn(x1,x2);
        tft_setRow(y1,y2);
        tft_SendCommand(0x2C);
        for(uint32_t i=0; i<=uint32_t((x2-x1+1)*(y2-y1+1));i++)
        {
            spi_sendWord(col);
        }
    }
};
class Line:public Pixel
{
public:
    Line(uint16_t x1=0,uint16_t y1=0,uint16_t x2=0,uint16_t y2=0,uint16_t col=BLACK)
    {
        //SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */ //FPU enable
        //checkXYswap(x1,y1,x2,y2);
        //if(x2!=x1)
        //{
        //    angle = atanf((y2-y1)/(x2-x1));
        //}
        //else if((x2==x1)&&y2!=y1)
        //{
        //    angle=90.0f*3.14159265f/90.0f ;
        //}
        //else {angle=0;}        
        length = float(sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))); //single float
        
        for(uint16_t i=0;i<uint32_t(length);i++) //uint16_t(length)
        {     
            //setPixel(x1+i,y1+sqrtf(i),col);
            setPixel(x1+uint16_t(i*((x2-x1)/length)),y1+uint16_t(i*((y2-y1)/length)),col);            
        }
    }
    void setLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t col)
    {
        float length1 = sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
        for(uint16_t i=0;i<uint32_t(length1);i++) //uint16_t(length)
        {     
            //setPixel(x1+i,y1+sqrtf(i),col);       
            setPixel(x1+i*(x2-x1)/length1,y1+i*(y2-y1)/length1,col);            
        }
    }
    void setDiscretLine(uint16_t X1,uint16_t Y1,uint16_t X2,uint16_t Y2,uint16_t col)
    {
        volatile uint16_t x1=X1; 
        volatile uint16_t y1=Y1;
        volatile uint16_t x2=X2;
        volatile uint16_t y2=Y2;   
        volatile uint16_t step = std::abs(y2-y1)>std::abs(x2-x1);
        if(step){swap(&x1,&y1);swap(&x2,&y2);} //меняем систему координат и отсчитываем в реальности по y
        //volatile uint16_t dx;
        //volatile uint16_t dy; 
        //dx=x2-x1; dy=std::abs(y2-y1);
        volatile uint16_t err=0;
        err = (x2-x1)/2; //точка перегиба
        volatile uint16_t ystep=0;
        if(y1<y2) {ystep=1;}
        else {ystep=-1;}     // если линия идет вверх меняем направление y

        for(;x1<=x2; x1++) //увеличиваем координату x1
        {
            if(step) {setPixel(y1,x1,col);} //если была поменяна система коодинат
            else     {setPixel(x1,y1,col);}
            err=err-(y2-y1);
            if(err>0)  //увеличиваем координату y
            {
                y1=y1+ystep+1;
                err=err+x2-x1;
            }
        }        
    }
private:
    float angle{0.0F};
    float length{0.0F}; //число точек - line length
    void swap(volatile uint16_t* x1,volatile uint16_t* x2)
    {
        volatile uint16_t z = *x2;*x1=*x2;*x2=z;
    }
};
class Elipse:public Pixel
{
public:
    Elipse(uint16_t x=0,uint16_t y=0,float R=0,uint16_t col=BLACK)
    {
        SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */ //FPU enable
        for (uint16_t i=0;i<=2*R;i++)        
        {
            setPixel(x+i,y+sqrtf(R*R - (i-R)*(i-R))+R,col);
            setPixel(x+i,y-sqrtf(R*R - (i-R)*(i-R))+R,col);
            setPixel(x+sqrtf(R*R - (i-R)*(i-R))+R,y+i,col);
            setPixel(x-sqrtf(R*R - (i-R)*(i-R))+R,y+i,col);
        }
    }
private:
    float R{0.f};
};

#endif //SPILCD_H_