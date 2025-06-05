#include <Arduino.h>
#include <SPI.h>

#define FFT 128
#define FF2 65

int bin[FF2] = { 0, 64, 32, 96, 16, 80, 48, 112, 8, 72, 40, 104, 24, 88, 56, 120, 4, 68, 36, 100,
                 20, 84, 52, 116, 12, 76, 44, 108, 28, 92, 60, 124, 2, 66, 34, 98, 18, 82, 50, 114, 10, 74, 42, 106,
                 26, 90, 58, 122, 6, 70, 38, 102, 22, 86, 54, 118, 14, 78, 46, 110, 30, 94, 62, 126, 1
               };
	       
int8_t window[FFT]={0,0,0,0,1,1,1,2,2,3,3,4,5,5,6,7,8,9,10,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,25,
                    26,27,27,28,29,29,29,30,30,31,31,31,31,31,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
                    32,32,32,32,32,32,32,32,32,32,32,32,31,31,31,31,31,30,30,29,29,29,28,27,27,26,25,25,24,23,22,
                    21,20,19,18,17,16,15,14,13,12,11,10,10,9,8,7,6,5,5,4,3,3,2,2,1,1,1,0,0,0,0};

int8_t f[FFT], f2[FFT], min, max;

int re[FFT+10], im[FFT], tr[FF2], ti[FF2];
int count = FFT/2;

const int AUDIO_IN = A7;

bool led_state = false;
int prev, x, y;

//--------------------------------------------------------------------------
void fft_level_0()
// all 63 twiddles get used at first stage, but no imag. components yet
{
  int a, b, tmp;

  //printf("level 0.......\n");
  tmp = re[0] + re[64]; re[64] = re[0] - re[64]; re[0] = tmp;
  a = 1; b = 65;
  // unroll all 9 twiddle options I think..........
  while (a < 6) { // (1,0):
    tmp = re[a] - re[b]; re[a] += re[b]; re[b] = tmp; im[b] = 0; a++; b++;
  }
  while (a < 15) { // (1,-half):
    tmp = re[a] - re[b]; re[a] += re[b]; re[b] = tmp; im[b] = -tmp / 2; a++; b++;
  }
  while (a < 18) { // (half,-half):
    tmp = re[a] - re[b]; re[a] += re[b]; tmp /= 2; re[b] = tmp; im[b] = -tmp; a++; b++;
  }
  while (a < 27) { // (half,-1):
    tmp = re[a] - re[b]; re[a] += re[b]; im[b] = -tmp; re[b] = tmp / 2; a++; b++;
  }
  while (a < 38) { // (0,-1):
    im[b] = re[b] - re[a]; re[a] += re[b]; re[b] = 0; a++; b++;
  }
  while (a < 47) { // (-half,-1):
    tmp = re[b] - re[a]; re[a] += re[b]; im[b] = tmp; re[b] = tmp / 2; a++; b++;
  }
  while (a < 50) { // (-half,-half):
    tmp = re[b] - re[a]; tmp /= 2; re[a] += re[b]; re[b] = tmp; im[b] = tmp; a++; b++;
  }
  while (a < 59) { // (-1,-half):
    tmp = re[b] - re[a]; re[a] += re[b]; re[b] = tmp; im[b] = tmp / 2; a++; b++;
  }
  while (a < 64) { // (-1,0):
    tmp = re[b] - re[a]; re[a] += re[b]; re[b] = tmp; im[b] = 0; a++; b++;
  }
}

//-------------------------------------------------------------------------
void fft_level_1()
// 31 twiddles used......
{
  int a, b, tmp, tmp2;

  //printf("level 1.......\n");
  tmp = re[0] + re[32]; re[32] = re[0] - re[32]; re[0] = tmp;

  a = 1; b = 33;
  // unroll all 9 twiddle options I think..........
  while (a < 3) { // (1,0):
    tmp = re[a] - re[b]; re[a] += re[b]; re[b] = tmp; im[b] = 0; a++; b++;
  }
  while (a < 8) { // (1,-half):
    tmp = re[a] - re[b]; re[a] += re[b]; re[b] = tmp; im[b] = -tmp / 2; a++; b++;
  }
  while (a < 9) { // (half,-half):
    tmp = re[a] - re[b]; re[a] += re[b]; tmp /= 2; re[b] = tmp; im[b] = -tmp; a++; b++;
  }
  while (a < 14) { // (half,-1):
    tmp = re[a] - re[b]; re[a] += re[b]; im[b] = -tmp; re[b] = tmp / 2; a++; b++;
  }
  while (a < 19) { // (0,-1):
    im[b] = re[b] - re[a]; re[a] += re[b]; re[b] = 0; a++; b++;
  }
  while (a < 24) { // (-half,-1):
    tmp = re[b] - re[a]; re[a] += re[b]; im[b] = tmp; re[b] = tmp / 2; a++; b++;
  }
  while (a < 25) { // (-half,-half):
    tmp = re[b] - re[a]; tmp /= 2; re[a] += re[b]; re[b] = tmp; im[b] = tmp; a++; b++;
  }
  while (a < 30) { // (-1,-half):
    tmp = re[b] - re[a]; re[a] += re[b]; re[b] = tmp; im[b] = tmp / 2; a++; b++;
  }
  while (a < 32) { // (-1,0):
    tmp = re[b] - re[a]; re[a] += re[b]; re[b] = tmp; im[b] = 0; a++; b++;
  }

  tmp = re[64] + re[96]; re[96] = re[64] - re[96]; re[64] = tmp;
  im[64] = im[96]; im[96] = -im[96];

  a = 65; b = 97;
  while (a < 96) { // 31 cycles of bft_2bit(a,b,c)
    tmp = re[a] - re[b];  re[a] += re[b];
    tmp2 = im[a] - im[b]; im[a] += im[b];

    if (a < 64 + 3) { // (1.0)
      re[b] = tmp; im[b] = tmp2;
    } else if  (a < 72) { // (1,-half)
      re[b] = tmp + tmp2 / 2; im[b] = tmp2 - tmp / 2;
    } else if (a < 73) { // (half, -half)
      re[b] = tmp / 2 + tmp2 / 2; im[b] = tmp2 / 2 - tmp / 2;
    } else if (a < 78) { // (half,-1)
      re[b] = tmp / 2 + tmp2; im[b] = tmp2 / 2 - tmp;
    } else if (a < 83) { // (0,-1)
      re[b] = tmp2; im[b] = -tmp;
    } else if (a < 88) { // (-half,-1)
      re[b] = tmp2 - tmp / 2; im[b] = -tmp2 / 2 - tmp;
    } else if (a < 89) { // (-half, -half)
      re[b] = tmp2 / 2 - tmp / 2; im[b] = -tmp2 / 2 - tmp / 2;
    } else if (a < 94) { // (-1, -half)
      re[b] = tmp2 / 2 - tmp; im[b] = -tmp2 - tmp / 2;
    } else { // (-1.0)
      re[b] = -tmp; im[b] = -tmp2;
    }
    a++; b++;
  }
}

//-------------------------------------------------------------------------
void fft_level_2()
// 15 twiddles used...
{
  int a, b, c, tmp, tmp2, x;

  //printf("level 2......\n");
  tmp = re[0] + re[16]; re[16] = re[0] - re[16]; re[0] = tmp;

  a = 1; b = 17; // 16 cycles of bft0(a,b,c)
  // unroll all 9 twiddle options I think..........
  while (a < 2) { // (1,0):
    tmp = re[a] - re[b]; re[a] += re[b]; re[b] = tmp; im[b] = 0; a++; b++;
  }
  while (a < 4) { // (1,-half):
    tmp = re[a] - re[b]; re[a] += re[b]; re[b] = tmp; im[b] = -tmp / 2; a++; b++;
  }
  while (a < 5) { // (half,-half):
    tmp = re[a] - re[b]; re[a] += re[b]; tmp /= 2; re[b] = tmp; im[b] = -tmp; a++; b++;
  }
  while (a < 7) { // (half,-1):
    tmp = re[a] - re[b]; re[a] += re[b]; im[b] = -tmp; re[b] = tmp / 2; a++; b++;
  }
  while (a < 10) { // (0,-1):
    im[b] = re[b] - re[a]; re[a] += re[b]; re[b] = 0; a++; b++;
  }
  while (a < 12) { // (-half,-1):
    tmp = re[b] - re[a]; re[a] += re[b]; im[b] = tmp; re[b] = tmp / 2; a++; b++;
  }
  while (a < 13) { // (-half,-half):
    tmp = re[b] - re[a]; tmp /= 2; re[a] += re[b]; re[b] = tmp; im[b] = tmp; a++; b++;
  }
  while (a < 15) { // (-1,-half):
    tmp = re[b] - re[a]; re[a] += re[b]; re[b] = tmp; im[b] = tmp / 2; a++; b++;
  }
  tmp = re[b] - re[a]; re[a] += re[b]; re[b] = tmp; im[b] = 0;

  im[32] = 0;
  a = 32; b = 48; c = 0;
  while (a < 128) { // 48 cycles of bft(a,b,c)...
    tmp = re[a] - re[b];  re[a] = re[a] + re[b];
    tmp2 = im[a] - im[b]; im[a] = im[a] + im[b];
    x = a & 15;
    if (x < 2) { // no twiddle
      re[b] = tmp; im[b] = tmp2;
    }  else if (x < 4) { // (1,-half)
      re[b] = tmp + tmp2 / 2; im[b] = tmp2 - tmp / 2;
    } else if (x < 5) { // (half, -half)
      re[b] = tmp / 2 + tmp2 / 2; im[b] = tmp2 / 2 - tmp / 2;
    } else if (x < 7) { // (half,-1)
      re[b] = tmp / 2 + tmp2; im[b] = tmp2 / 2 - tmp;
    } else if (x < 10) { // (0,-1)
      re[b] = tmp2; im[b] = -tmp;
    } else if (x < 12) { // (-half,-1)
      re[b] = tmp2 - tmp / 2; im[b] = -tmp2 / 2 - tmp;
    } else if (x < 13) { // (-half, -half)
      re[b] = tmp2 / 2 - tmp / 2; im[b] = -tmp2 / 2 - tmp / 2;
    } else if (x < 15) { // (-1, -half)
      re[b] = tmp2 / 2 - tmp; im[b] = -tmp2 - tmp / 2;
    } else { // (-1.0)
      re[b] = -tmp; im[b] = -tmp2; a += 16; b += 16;
    }
    a++; b++;
  }
}

//-------------------------------------------------------------------------
void fft_level_3()
// 7 twiddles in play, 8,16,24,32,40,48,56.
{
  int a, b, x, tmp, tmp2;

  //printf("level 3......\n");
  tmp = re[0] + re[8]; re[8] = re[0] - re[8]; re[0] = tmp;
  for (a = 1; a < 17; a++) im[a] = 0; // just set missing imag components now (15)
  a = 1; b = 9;
  while (a < 128) { // 56 cycles of bft(a,b,c)
    tmp = re[a] - re[b];  re[a] = re[a] + re[b];
    tmp2 = im[a] - im[b]; im[a] = im[a] + im[b];
    x = a & 7;
    if (x == 0) {
      re[b] = tmp;  // (no twiddle)
      im[b] = tmp2;
    }
    if (x == 1) {
      re[b] = tmp + tmp2 / 2;  // (1,-half)
      im[b] = tmp2 - tmp / 2;
    }
    if (x == 2) {
      re[b] = tmp / 2 + tmp2 / 2;  // (half, -half)
      im[b] = tmp2 / 2 - tmp / 2;
    }
    if (x == 3) {
      re[b] = tmp / 2 + tmp2;  // (half,-1)
      im[b] = tmp2 / 2 - tmp;
    }
    if (x == 4) {
      re[b] = tmp2;  // (0,-1)
      im[b] = -tmp;
    }
    if (x == 5) {
      re[b] = tmp2 - tmp / 2;  // (-half,-1)
      im[b] = -tmp2 / 2 - tmp;
    }
    if (x == 6) {
      re[b] = tmp2 / 2 - tmp / 2;  // (-half, -half)
      im[b] = -tmp2 / 2 - tmp / 2;
    }
    if (x == 7) {
      re[b] = tmp2 / 2 - tmp;  // (-1, -half)
      im[b] = -tmp2 - tmp / 2;
      a += 8;
      b += 8;
    }
    a++; b++;
  }
}

//-------------------------------------------------------------------------
void fft_level_4()
// 3 twiddles in play, 16 (half,-half),32 (0,-1) ,48 (-half,-half)
{
  int a, b, tmp, tmp2, x;

  //printf("level 4......\n");
  tmp = re[0] + re[4]; re[4] = re[0] - re[4]; re[0] = tmp;
  tmp = re[1] - re[5]; re[1] = re[1] + re[5];
  im[5] = tmp * ti[16] / 2; re[5] = tmp * tr[16] / 2;
  im[6] = re[6] - re[2]; re[2] = re[2] + re[6]; re[6] = 0;
  tmp = re[3] - re[7]; re[3] = re[3] + re[7];
  im[7] = tmp * ti[48] / 2; re[7] = tmp * tr[48] / 2;
  im[8] = 0;
  a = 8; b = 12;
  while (a < 128) { // 60 cycles of bft(a,b,c)
    x = a & 3;
    if (x == 2) {
      tmp = re[b] - re[a]; tmp2 = im[a] - im[b];
      re[a] += re[b]; im[a] += im[b]; re[b] = tmp2; im[b] = tmp;
    } else {
      tmp = re[a] - re[b];  re[a] = re[a] + re[b];
      tmp2 = im[a] - im[b]; im[a] = im[a] + im[b];
    }
    if (x == 0) {
      re[b] = tmp;
      im[b] = tmp2;
    }
    if (x == 1) {
      re[b] = tmp / 2 + tmp2 / 2;
      im[b] = tmp2 / 2 - tmp / 2;
    }
    if (x == 3) {
      re[b] = tmp2 / 2 - tmp / 2;
      im[b] = -tmp2 / 2 - tmp / 2;
      a += 4;
      b += 4;
    }
    a++; b++;
  }
}

//-----------------------------------------------------------------------
void fft_levels_5_6()
// level 5 = only twiddle 32 (tr= 0, ti= -1), hugely simplifies this
// level 6 = no twiddles, just butterfly, ony require even bins and bin 1.
{
  int a, b, c, d, tmp;

  //printf("levels 5&6....\n");
  // bf0(0,2), bft0(1,3), bf0(0,1),bf1(2,3). Afterwards, ignore bins 0,3....
  im[2] = re[3] - re[1];
  re[1] = abs(re[1] + re[3] - re[0] - re[2]);
  re[2] = re[0] - re[2];

  // bf1(4,6),bft32(5,7),bf(4,5),bf(6,7). Afterwards, ignore bins 5,7
  im[4] = im[6] + im[5]; im[4] += im[7];
  im[6] = re[7] - im[6]; im[6] -= re[5];
  tmp = re[4] + re[6]; re[6] = re[4] - re[6];
  re[4] = tmp + re[5]; re[4] += re[7];
  re[6] += im[5]; re[6] -= im[7];

  a = 8; b = 9; c = 10; d = 11;
  while (a < 128) {
    tmp = re[a] + re[c]; re[c] = re[a] - re[c];
    re[a] = tmp + re[b]; re[a] += re[d];
    re[c] += im[b];  re[c] -= im[d];

    tmp = im[a] + im[c]; im[c] = im[a] - im[c];
    im[a] = tmp + im[b]; im[a] += im[d];
    im[c] += re[d]; im[c] -= re[b];
    a += 4; b += 4; c += 4; d += 4;
  }
}

//-----------------------------------------------------------------------------------
void setup() {
  Serial.begin(230400);
  pinMode(LED_BUILTIN, OUTPUT);  // Set pin 12 (LED) as output

  // set up timer0 interrupt at 6.4KHz....
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 38; // = (16*10^6) / (6400*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);


  pinMode(AUDIO_IN, INPUT);
  //analogReference(EXTERNAL);
  analogRead(AUDIO_IN); // initialise ADC to read audio input

  sei();  // Enable back the interrupts
}

//-------------------------------------------------------------------------------------
// The Interrupt-service routine (ISR).
// With the settings above, this ISR will trigger 6400 times/sec...
ISR(TIMER0_COMPA_vect) {
  TCNT0 = 0;  // Reset the counter
  // read the ADC and dump level to serial...
  int i = ADCL;
  i += ADCH << 8;
  x = i - prev; prev = i;  y = x;
  //bitSet(ADCSRA, ADIF); // clear the flag
  
  if (count==0 || count>=FFT) {
    count=0;
    Serial.println("oops!");
  } else {
    f[count] = y; count++;
    //Serial.write(y);
    bitSet(ADCSRA, ADSC); // start next ADC conversion
  }
}


//-------------------------------------------------------------------------------------
void loop() {
  uint8_t i, j, n,b,top;
  int max3;

  // check for next 64 samples having been read (10ms passed)....
  if (count >= FFT) { // scale and window frame, run an FFT,
    count = FFT / 2;  // rewind frame counter 64 samples
    // get min/max of f buffer....
    min = 127; max = -127;
    for (i = 0; i < FFT; i++) {
      if (f[i] < min) min = f[i];
      if (f[i] > max) max = f[i];
    }
    //Serial.print(min); Serial.print(" , "); Serial.print(max); Serial.print("\n");
    if (-min > max) max = -min;
    int8_t  div = 1; while (max > 8) {max /= 2; div *= 2; }
    for (i = 0; i < FFT; i++) f2[i] = f[i] / div; // scale down the frame data
    Serial.print(div); Serial.print(",");

    for (i = 0; i < FFT; i++) re[i] = f2[i]*window[i];
    for (i = 0, j = FFT / 2; j < FFT; i++, j++) f[i] = f[j]; // shift samples down...

    fft_level_0();
    fft_level_1();
    fft_level_2();
    fft_level_3();
    fft_level_4();
    fft_levels_5_6();
    
    // now read the FFT output (backwards).... 
    for (i=64, n=127; i>0; i--, n-=2) {
      b=bin[i];
      if (re[b]<0) re[b]=-re[b];
      if (im[b]<0) im[b]=-im[b];
      // This is a 16bit heuristic of relative magnitude of re*re+im*im....
      if (re[b]>im[b]) re[n]=re[b]+im[b]/2; else re[n]=im[b]+re[b]/2;
    }
    // now find the top 5 bins....
    for (i=0; i<5; i++) {
      max3=0; for (n=1; n<127; n+=2) if (re[n]>max3) {max3=re[n]; top=n;}
      b=bin[top/2+1];  // should this me just top/2????
      Serial.print(top/2+1); Serial.print(",");
      Serial.print(re[b]); Serial.print(","); Serial.print(im[b]); Serial.print(",");
      //Serial.print(top); Serial.print(",");
      re[top]=0;
      // blank adjacent bins kinda mel-scale-style...
      re[top+2]=0;
      if (top>1)  re[top-2]=0;
      if (top>20) {re[top+4]=0; re[top-4]=0;}
      if (top>40) {re[top+6]=0; re[top-6]=0;}
      if (top>60) {re[top+8]=0; re[top-8]=0;}
    }
    Serial.print("\n");
  }
}
