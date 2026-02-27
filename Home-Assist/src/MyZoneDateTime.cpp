
#include "global.h"
#include "MyDateTime.h"
#include "MyZoneDateTime.h"
#include "MyConfig.h"

//
// Retour
// 0 : RAS
//
int CZoneDateTime::drawDateTime(const String& date, const String& heure) { 

  mTft.fillRect(muiPosX, muiPosY, muiWidth, muiHight, muiBgColor);  
  mTft.setTextColor(DATE_HEURE_OK_C);
  mTft.drawString(date, muiDateX, muiDateY, muiDateFont);
  mTft.drawString(heure, muiTimeX, muiTimeY, muiTimeFont);
  return 0;

}

//
// Retour
// 0 : RAS
// -1 : Pointeur non alloué
//
int CZoneDateTime::drawDateTime() { 
  if (mConfig == nullptr) return -1;
  if (mConfig->mDateTime == nullptr) return -1;

  String sDate, sHeure;
  CMyDateTime& mDateTime = *mConfig->mDateTime;
  sDate = (mDateTime.getDate()).substring(0, 5);
  sHeure = mDateTime.getTime().substring(0, 5);
  drawDateTime(sDate, sHeure);

  return 0;

}
