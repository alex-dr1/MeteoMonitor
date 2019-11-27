#include "unixTimeConv.h"

#define LEAP_YEAR_DAYS 366
#define YEAR_DAYS 365
#define DAY 86400

int monDay(int *fdays, int leapYear){
  int dayMonth[12] = 
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (leapYear) dayMonth[1] = 29;
  for (int i = 1; i < (int)sizeof(dayMonth); ++i){
    if (*fdays<=dayMonth[i-1]){
      ++*fdays;
      return i;
    } 
    *fdays = *fdays - dayMonth[i-1];
  }
  return 0;
}


char *unixTimeConv(long epoch, int UTC)
{
  int second, minute, hour, day, month, year;
	int days4Years;
  char digChar[12] = "0123456789";
  char *timeDate;
  timeDate = "00:00:00 00.00.0000";
  
	epoch = epoch+(UTC*60*60);       
	second = epoch % 60;
	minute = (epoch/60)%60;
	hour = (epoch/(60*60))%24;
  days4Years = (epoch/(DAY))-(YEAR_DAYS*2+(((epoch/(DAY))-YEAR_DAYS*2)/(LEAP_YEAR_DAYS+YEAR_DAYS*3))*(LEAP_YEAR_DAYS+YEAR_DAYS*3));

  if (days4Years<=LEAP_YEAR_DAYS)
  {
    year = 1970+(2+(((epoch/(DAY))-YEAR_DAYS*2)/(LEAP_YEAR_DAYS+YEAR_DAYS*3))*4);
    day = (epoch/(DAY))-(YEAR_DAYS*2+(((epoch/(DAY))-YEAR_DAYS*2)/(LEAP_YEAR_DAYS+YEAR_DAYS*3))*(LEAP_YEAR_DAYS+YEAR_DAYS*3));
    month = monDay(&day, 1);
   } else {
    year = 1970+((2+(((epoch/(DAY))-YEAR_DAYS*2)/(LEAP_YEAR_DAYS+YEAR_DAYS*3))*4))+((days4Years-LEAP_YEAR_DAYS)/YEAR_DAYS)+1;
    day = (days4Years-LEAP_YEAR_DAYS)-(((days4Years-LEAP_YEAR_DAYS)/YEAR_DAYS)*YEAR_DAYS);
    month = monDay(&day, 0); 
   }

    //hour, minute, second, day, month, year;
    

   timeDate[0] = digChar[hour/10];
   timeDate[1] = digChar[hour-(hour/10)*10];

   timeDate[3] = digChar[minute/10];
   timeDate[4] = digChar[minute-(minute/10)*10];

   timeDate[6] = digChar[second/10];
   timeDate[7] = digChar[second-(second/10)*10];

   timeDate[9] = digChar[day/10];
   timeDate[10] = digChar[day-(day/10)*10];

   timeDate[12] = digChar[month/10];
   timeDate[13] = digChar[month-(month/10)*10];

   timeDate[15] = digChar[year/1000];
   timeDate[16] = digChar[(year-(year/1000)*1000)/100];
   timeDate[17] = digChar[(year-(year/100)*100)/10];
   timeDate[18] = digChar[(year-(year/100)*100)-((year-(year/100)*100)/10)*10];

   return timeDate;
    
}

