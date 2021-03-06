//#include <ArduiPi_SSD1306.h>
//#include <Adafruit_SSD1306.h>
#include <ArduiPi_OLED_lib.h>
#include <ArduiPi_OLED.h>

#include <Adafruit_GFX.h>
#include <math.h>
#include <time.h>
#include <sys/statvfs.h>
 
#define X0 0
#define Y0 1
#define X1 2
#define Y1 3
#define PI 3.14159265
 
typedef struct {
  // Struct, um die Koordinaten aller Zeiger-Positionen
  // für die drei Zeiger (Sekunden, Minuten, Stunden)
  // vorauszuberechnen
  uint16_t x0;
  uint16_t y0;
  uint16_t sek[60][2];
  uint16_t min[60][2];
  uint16_t std[12][2];
} zeiger_werk;
 
void initZeiger(uint16_t x, uint16_t y, uint16_t r, zeiger_werk *zeiger) {
  // Berechnet für einen Mittelpunkt und einen Radius alle Zeigerkoordinaten
  zeiger->x0 = x;
  zeiger->y0 = y;
  for(uint8_t i=0;i<60;i++) {
    float alpha = 6.0*i*PI/180.0 - PI/2.0;
    float cosinus = cos(alpha);
    float sinus = sin(alpha);
    zeiger->sek[i][0] = r * cosinus + x;
    zeiger->sek[i][1] = r * sinus + y;
    zeiger->min[i][0] = (r - 5) * cosinus + x;
    zeiger->min[i][1] = (r - 5) * sinus + y;
    if(i % 5 == 0) {
      zeiger->std[i/5][0] = (r - 10) * cosinus + x;
      zeiger->std[i/5][1] = (r - 10) * sinus + y;
    }
  }
}
 
//Adafruit_SSD1306 display;
// Instantiate the display
ArduiPi_OLED display;

 
void drawClock(uint16_t x, uint16_t y, uint16_t r, uint8_t tick_length) {
 
  display.drawCircle(x, y, r, WHITE);
 
  for (uint8_t i=0;i<12;i++) {
    double alpha = 30.0*i*PI/180.0;
    uint16_t x1 = (r - tick_length) * cos(alpha) + x;
    uint16_t y1 = (r - tick_length) * sin(alpha) + y;
    uint16_t x2 = r * cos(alpha) + x;
    uint16_t y2 = r * sin(alpha) + y;
    display.drawLine(x1,y1,x2,y2,WHITE);
  }
 
  display.display();
}
 
void drawSeconds(zeiger_werk zeiger, uint8_t sec, uint16_t color)
{
  // zeichnet für einen Sekundenwert den Sekundenzeiger mit
  // Hilfe der abgespeicherten Zeigerkoordinaten
  display.drawLine(zeiger.x0, zeiger.y0, zeiger.sek[sec][0],zeiger.sek[sec][1], color);
  display.display();
}
 
void drawMinutes(zeiger_werk zeiger, uint8_t min, uint16_t color)
{
  // zeichnet für einen Minutenwert den Minutenzeiger mit
  // Hilfe der abgespeicherten Zeigerkoordinaten
  display.drawLine(zeiger.x0, zeiger.y0, zeiger.min[min][0],zeiger.min[min][1], color);
  display.display();
}
 
void drawHours(zeiger_werk zeiger, uint8_t std, uint16_t color)
{
  // zeichnet für einen Stundenwert den Stundenzeiger mit
  // Hilfe der abgespeicherten Zeigerkoordinaten, bildet dabei
  // 24-Stundenangaben auf 12-Stunden der analogen Uhr ab
  display.drawLine(zeiger.x0, zeiger.y0, zeiger.std[std % 12][0],zeiger.std[std % 12][1], color);
  display.display();
}
 
void drawZeit(const uint16_t x0, const uint16_t y0, const uint16_t zeile_len, struct tm *zeit)
{
  char uhrzeit[9];
  strftime(uhrzeit,9,"%H:%M:%S", zeit);
  display.fillRect(x0,y0, zeile_len, 8,BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(x0,y0);
  display.print(uhrzeit);
  display.display();
}
 
void drawWochentag(const uint16_t x0, const uint16_t y0, const uint16_t zeile_len, struct tm *zeit)
{
  const char *wday[] = { "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag", "???" };
  display.fillRect(x0,y0, zeile_len, 8,BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(x0,y0);
  display.print(wday[zeit->tm_wday]);
  display.display();
}
 
void drawDatum(const uint16_t x0, const uint16_t y0, const uint16_t zeile_len, struct tm *zeit)
{
  char datum[11];
  strftime(datum,11,"%d.%m.%y", zeit);
  display.fillRect(x0,y0, zeile_len, 8,BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(x0,y0);
  display.print(datum);
  display.display();
}
 
void drawCPUTemp(const uint16_t x0, const uint16_t y0, const uint16_t zeile_len)
{
  FILE *temperatureFile;
  double T = 0.0;
 
  temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
  if (temperatureFile) {
   fscanf (temperatureFile, "%lf", &T);
   fclose (temperatureFile);
  }
  T = T/1000.0;
  display.fillRect(x0,y0, zeile_len, 8,BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(x0,y0);
  display.printf("CPU: %.0fC",T);
  display.display();
}
 
void drawDiskUsage(const uint16_t x0, const uint16_t y0, const uint16_t zeile_len)
{
  struct statvfs buf;
  double usage = 0.0;
 
  if (!statvfs("/etc/rc.local", &buf)) {
    unsigned long hd_used;
    hd_used = buf.f_blocks - buf.f_bfree;
    usage = ((double) hd_used) / ((double) buf.f_blocks) * 100;
  }
 
  display.fillRect(x0,y0, zeile_len, 8,BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(x0,y0);
  display.printf("HD: %.0f%%",round(usage));
  display.display();
}
 
int main(int argc, char **argv)
{
  time_t unix_sek;
  struct tm *zeit;
  uint8_t sekunden;
  uint8_t minuten;
  uint8_t stunden;
  zeiger_werk zeiger;
 
  if ( !display.init(OLED_I2C_RESET,OLED_ADAFRUIT_I2C_128x64) )
    exit(EXIT_FAILURE);
 
  display.begin();
 
  long clock_center[] =  { display.width()/2 - 32, display.height()/2 };
  const uint16_t RADIUS = display.height()/2-1;
  const uint16_t ZEIGER_SEK_LEN = display.height()/2-5;
  const uint16_t TEXT_AREA_START_X = display.width()/2;
 
  initZeiger(clock_center[0],clock_center[1],ZEIGER_SEK_LEN, &zeiger);
 
  display.clearDisplay();
 
  drawClock(clock_center[X0], clock_center[Y0], RADIUS, 3);
 
  unix_sek = time(NULL);
  zeit = localtime(&unix_sek);
  sekunden = zeit->tm_sec;
  minuten = zeit->tm_min;
  stunden = zeit->tm_hour;
  drawSeconds(zeiger, zeit->tm_sec, WHITE);
  drawMinutes(zeiger, zeit->tm_min, WHITE);
  drawHours(zeiger, zeit->tm_hour, WHITE);
 
  while (1) {
    unix_sek = time(NULL);
    zeit = localtime(&unix_sek);
    if (zeit->tm_sec != sekunden) {
      drawSeconds(zeiger, sekunden, BLACK);
      if (zeit->tm_min != minuten) {
        drawMinutes(zeiger, minuten, BLACK);
        minuten = zeit->tm_min;
      }
      if (zeit->tm_hour != stunden) {
        drawHours(zeiger, stunden, BLACK);
        minuten = zeit->tm_hour;
      }
      drawHours(zeiger, zeit->tm_hour, WHITE);
      drawMinutes(zeiger, zeit->tm_min, WHITE);
      drawSeconds(zeiger, zeit->tm_sec, WHITE);
      drawZeit(TEXT_AREA_START_X + 10, 5, 54, zeit);
      drawWochentag(TEXT_AREA_START_X + 10, 15, 54, zeit);
      drawDatum(TEXT_AREA_START_X + 10, 25, 54, zeit);
      drawCPUTemp(TEXT_AREA_START_X + 10, 45, 54);
      drawDiskUsage(TEXT_AREA_START_X + 10, 55, 54);
      sekunden = zeit->tm_sec;
    }
    delay(300);
  }
 
  display.close();
}
