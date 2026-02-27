#ifndef __RCSWITCHCODEANALYZER__
#define __RCSWITCHCODEANALYZER__
#include <Arduino.h>

//=======================================================================================
// Traitement des codes RCSwitch
//=======================================================================================
struct STRUCT_RCS_HEADER {
  unsigned int reserved : 12;
  unsigned int nbMesures : 4;
  unsigned int id : 8;
};
struct STRUCT_RCS_MESURE {
  unsigned int val : 8;
  unsigned int type : 4;
  unsigned int numero : 4;
  unsigned int id : 8;
};
struct STRUCT_RCS_FOOTER {
  unsigned int reserved : 8;
  unsigned int crc8 : 8;
  unsigned int id : 8;
};

struct _ST_MESURE_ {
    int type;
    float val;
};
class CRCSwitchCodeAnalyzer {
    private:
    public:
        bool decodeHeader(STRUCT_RCS_HEADER& st, unsigned long code);
        bool decodeMesure(STRUCT_RCS_MESURE& st, unsigned long code);
        bool decodeFooter(STRUCT_RCS_FOOTER& st, unsigned long code);
        bool decode(_ST_MESURE_ *stm, unsigned long *tab, unsigned char taille);
        void printStructRCSMesure(STRUCT_RCS_MESURE stm);
        void printStructSTSMesure(_ST_MESURE_ stm);
        void printStructFooter(STRUCT_RCS_FOOTER &stf);
        uint8_t computeCRC(const unsigned long* codes, int count);
};
#endif // __RCSWITCHCODEANALYZER__
