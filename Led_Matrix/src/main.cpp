#include <Arduino.h>
#include "MD_MAX72xx.h"
#include "FontKeszlet.h"
#include <avr/interrupt.h>

#define KijelzokSzama 8
#define KijelzoMeret 8
#define SorokSzama 16
#define OszlopokSzama 32
#define PinDATA 8
#define PinCS 9
#define PinCLK 10
#define Intenzitas 4
#define KaratkerKoz 1
#define MaxKarakter 200
#define MegjelenitesSora 5

char szoveg[MaxKarakter + 1] = {'\0'};
int16_t pozicio;
int16_t matrixSzovegHossz;
int16_t szovegHossz;

byte megjelenitesSzamlalo = 0;
byte megjelenitesPeriodus = 5;

char szovegBuffer[MaxKarakter + 1] = {'\0'};
uint16_t beolvasottIndex = 0;

byte potSzamlalo = 0;
byte potPeriodus = 100;

MD_MAX72XX max = MD_MAX72XX(MD_MAX72XX::moduleType_t::GENERIC_HW, PinDATA, PinCLK, PinCS, KijelzokSzama);

void allitLed(int16_t sor, int16_t oszlop, bool ertek)
{
  if (sor >= 0 && sor < SorokSzama && oszlop >= 0 && oszlop < OszlopokSzama)
  {
    max.setPoint(7 - (oszlop & 7), ((sor >> 3) << 5) + ((oszlop >> 3) << 3) + 7 - (sor & 7), ertek);
  }
}
void allitTerulet(int16_t sorMettol, int16_t sorMeddig, int16_t oszlopMettol, int16_t oszlopMeddig, bool ertek)
{
  for (int i = sorMettol; i <= sorMeddig; i++)
  {
    for (int j = oszlopMettol; j <= oszlopMeddig; j++)
    {
      allitLed(i, j, ertek);
    }
  }
}
void allitOszlop(int16_t oszlop, int16_t ertek)
{
  if (oszlop >= 0 && oszlop < OszlopokSzama)
  {
    for (size_t i = 0; i < SorokSzama; i++)
    {
      allitLed(i, oszlop, ertek);
    }
  }
}
void allitSor(int16_t sor, int16_t ertek)
{
  if (sor >= 0 && sor < SorokSzama)
  {
    for (size_t i = 0; i < OszlopokSzama; i++)
    {
      allitLed(sor, i, ertek);
    }
  }
}
void kiirKarakter(unsigned char karakter, int16_t sor, int16_t oszlop)
{
  if (karakter > KarakterekDarab)
  {
    karakter = 0;
  }

  for (int i = 0; i < FontMeretY; i++)
  {
    for (int j = 0; j < FontMeretX; j++)
    {

      allitLed(sor + i, oszlop + j, FontKeszlet[karakter][i] & (128 >> j));
    }
  }
}

void kiirSzoveg()
{
  max.update(MD_MAX72XX::controlValue_t::OFF);
  max.clear();

  for (int16_t j = 0; j < szovegHossz; j++)
  {
    int16_t kezdet = pozicio + j * (FontMeretX + KaratkerKoz);
    int16_t vege = kezdet + FontMeretX;
    if (vege >= 0 && kezdet <= OszlopokSzama)
    {
      kiirKarakter(szoveg[j], MegjelenitesSora, kezdet);
      allitTerulet(MegjelenitesSora, MegjelenitesSora + FontMeretY, vege, vege + KaratkerKoz, false);
    }
  }

  max.update(MD_MAX72XX::controlValue_t::ON);

  if (pozicio > -matrixSzovegHossz)
  {
    pozicio--;
  }
  else
  {
    pozicio = OszlopokSzama;
  }
}

void beallitSzoveg()
{
  pozicio = OszlopokSzama;

  int16_t i = 0;
  while (i < MaxKarakter && szovegBuffer[i] != '\0')
  {
    szoveg[i] = szovegBuffer[i];
    i++;
  }

  szovegHossz = i;
  matrixSzovegHossz = szovegHossz * (FontMeretX + KaratkerKoz);
}

void periodusAllitas()
{

  uint16_t ertek = analogRead(A5) / 100 + 1;
  if (megjelenitesPeriodus != ertek)
  {
    megjelenitesPeriodus = ertek;
  }
}
void setup()
{
  Serial.begin(9600);

  TCCR2A = (1 << WGM21);
  // CS22 CS21 CS20 előosztás
  //   0   0   0      -
  //   0   0   1     /1
  //   0   1   0     /8
  //   0   1   1     /32
  //   1   0   0     /64
  //   1   0   1     /128
  //   1   1   0     /256
  //   1   1   1     /1024
  //  16 mhz/(1024*(155+1))=100 hz ->0.01 sec
  TCCR2B = (1 << CS20) | (1 << CS21) | (1 << CS22);
  OCR2A = 155;
  TIMSK2 = (1 << OCIE2A);
  sei();

  max.begin();
  max.control(MD_MAX72XX::controlRequest_t::INTENSITY, 1);
}

void megszakitas(byte *szamlalo, byte *periodus, void (*eljaras)())
{
  if (*szamlalo < *periodus)
  {
    *szamlalo = *szamlalo + 1;
  }
  else
  {
    eljaras();
    *szamlalo = 0;
  }
}

ISR(TIMER2_COMPA_vect)
{
  megszakitas(&potSzamlalo, &potPeriodus, periodusAllitas);
  megszakitas(&megjelenitesSzamlalo, &megjelenitesPeriodus, kiirSzoveg);
}

void loop()
{
  if (Serial.available())
  {
    char k = Serial.read();
    Serial.write(k);

    if (beolvasottIndex >= 0 && k == 8)
    {
      szovegBuffer[beolvasottIndex] = '\0';
      if (beolvasottIndex > 0)
      {
        beolvasottIndex--;
      }
    }
    else if (beolvasottIndex < MaxKarakter && k != '\r' && k != '\n')
    {
      szovegBuffer[beolvasottIndex] = k;
      beolvasottIndex++;
    }
    else if (beolvasottIndex == MaxKarakter || k == '\n')
    {
      if(beolvasottIndex==MaxKarakter)
      {
        Serial.println();
      }
      szovegBuffer[beolvasottIndex] = '\0';
      beallitSzoveg();
      beolvasottIndex = 0;
    }
  }
}
