/*-------------------==========-------------------

                HV5812 DRIVER FOR ARDUNIO (MINI PRO)
              original PCB from samsumg ER-350F cashmachine

              this program is driver for the HV5812 over SPI interface,
              using arudnio mini pro, but it should work with any other 
              type of this microcontroller family.

              after starting up it should count from 00.00.00 up to 23.59.59, then reset ofc, 
              in one second intervals, using the timer1 interrupt divided to 1 Hz

              this code is free to use and can be altered without any permission from the owner
              ((its kinda shitty, feel free to improve))
*/
              


#include <SPI.h>        // include the spi module


#define DATAOUT 11//MOSI, connect this to the data input of the HV5812
#define SPICLOCK  13//clock for the SPI, connect this to the clk input of the HV5812
const int ssPin = 10; //slave select, this should be connected to the LATCH pin of the HV51812. can be any pin on the arduino.


void setup()

{

  // configure the used pins
  pinMode(DATAOUT, OUTPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(ssPin, OUTPUT);
  digitalWrite(ssPin, LOW);  // just to make sure

  

  // initialize timer1 

  noInterrupts();           // disable all interrupts

  TCCR1A = 0;

  TCCR1B = 0;

  TCNT1  = 0;


  OCR1A = 31250;            // compare match register 16MHz/256/2Hz

  TCCR1B |= (1 << WGM12);   // CTC mode

  TCCR1B |= (1 << CS12);    // 256 prescaler 

  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

  interrupts();             // enable all interrupts

}

void init_SPI(){
  // initialize the SPI communication
  SPI.begin();  // Begin SPI hardware
  SPI.setClockDivider(SPI_CLOCK_DIV2);  // Slow down SPI clock, can be 2,4,8,... etc, should work with all of them
  SPI.setBitOrder(LSBFIRST);            // order of bits on the serial data output
  SPI.setDataMode(SPI_MODE0);           //spi mode select

}

// global variables for the clock
int sec = 0;          
int tensec = 0;
int minute = 0;
int tenminute = 0;
int hour = 0;
int tenhour = 0;
bool count = false;       // needed to divide the 2Hz from the timer1 interrupt down to 1Hz



ISR(TIMER1_COMPA_vect){    //interrupt for the clock

count = count^1;            //dividing down to 1Hz

if (count){
    
    sec++;
    if(sec > 9)               // a basic function for the clock
    {
      sec = 0;
      tensec++;
      if (tensec > 5)
      {
        minute++;
        tensec = 0;
      }
      if(minute >9)
      {
        tenminute++;
        minute = 0;
      }
      if(tenminute > 5)
      {
        tenminute = 0;
        hour ++;
      }
      if (hour > 9)
      {
        tenhour++;
        hour = 0;
      }
      if (tenhour >2)
      {
        tenhour = 0;
      }
    }
  }
    

}

char tx(char number, bool strobe)   // this function is responsible for the SPI communication, and the slaveSelect pin
                                    // bool strobe = if true, a short strobe impulse is given at the ssPin, wich basicly tell the
                                    //hv5812 to display the data.
{

  
  init_SPI();                     //initializing the spi interface
  SPI.transfer(number);           //transfering the input parameter "char number"

  
  if (strobe == true){            //this is for the strobe impulse as descripbed earlier
    digitalWrite(ssPin, HIGH);
    delayMicroseconds(0.5);
    digitalWrite(ssPin, LOW);
    delayMicroseconds(1);
   
  }
  
  
}

char symbols(char symbol, char digit)   // function for searching the bits belonging to each number and selected digit.
                                        //char symbol: the desired symbol (number or decimal point) to be displayed
                                        //char digit: the number of digit where the symbols should be displayed
{
  char symbols[] = {0,         1,          2,         3,          4,          5,          6,          7,         8,           9,          'p'         };  //each symbols has its own bits underneath this line, p =decimal point
                //  A_0                                                                                                                                     each bit represents the segment above it, if 1, the segment will be lit.
  char first[] = {0b10000000, 0b00000000, 0b10000000, 0b10000000, 0b00000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b00000000};    //alter bits in each byte to change the look of the symbols
                //  54321DCB                                                                                                                              // numbers represent digits, letter are for segments
  char second[]= {0b00000111, 0b00000011, 0b00000101, 0b00000111, 0b00000011, 0b00000110, 0b00000110, 0b00000011, 0b00000111, 0b00000111, 0b00000000};    //you can add more symbols to this list
                //  FGEp9876
  char third[]= { 0b10100000, 0b00000000, 0b01100000, 0b01000000, 0b11000000, 0b11000000, 0b11100000, 0b00000000, 0b11100000, 0b11000000, 0b00010000};

  for (int i = 0; i<=sizeof(symbols);i++)
  {
    if (symbols[i] == symbol)
    {

      tx(first[i],false);                       //transmitting the first byte of the symbols

      if (digit<5)                              //as the content fo the second or third byte depends on where it shoul be displayed, 
      {                                         //this part adds the neccessary high bit to each byte, accoring to the input paramater "digit"
      tx(second[i]xor(0b1<<digit+3),false);
      tx(third[i], true);
      }
      else
      {
      tx(second[i],false);
      tx(third[i]xor(0b1<<(digit-5)), true);  // set strobe to true, if done with one character's data
      }
      break;
    }
  }
  
}

void loop()

{                                   //the main loop
  if (count){symbols('p',4);
              symbols('p',6);
         symbols(sec,2);}
  else{  symbols(sec,2);}
  //delay(1);}
  symbols(tensec,3);
  
  symbols(minute,4);
  symbols(tenminute,5);
  
  symbols(hour,6);
  symbols(tenhour,7);

  //symbols(what,where); use like this

}
