#ifndef __MYRADIOTX_H__
#define __MYRADIOTX_H__

#define DELAI_AVANT_ENVOI_2 6000UL
#define DELAI_AVANT_ENVOI_3 8000UL
//#define NB_ENVOIS 3

class CRadioTX {
private:
  CConfig& config;
  CEcran* ecran;
  bool mbEnvoyerTramesON;
  bool mbEnvoyerTramesOFF;
  int miCompteurEnvois;
  int muiKeepAliveIndexEnCours = 0; // Index du keep-alive anvoyé
  bool mbKeepAliveActive = false;
  bool mbTrameCommandeEnvoyee = false; // Premiere trame ON ou OFF déjà envoyée


public:
  CRadioTX(CEcran* ptrEcran, CConfig& cfg) : ecran(ptrEcran), config(cfg) {
    mbEnvoyerTramesON = false; mbEnvoyerTramesOFF = false; miCompteurEnvois=0;
  }

  void setup();
  void loop();
  int isetUpStatus(bool bStatus);
  void transmitPulses(const uint16_t* pulses, int nbPulses, const char* action);
  bool bSetEnvoyerTramesON(bool b);
  bool bSetEnvoyerTramesOFF(bool b);
  bool bGetEnvoiEnCours(void);
};
#endif // __MYRADIOTX_H__
