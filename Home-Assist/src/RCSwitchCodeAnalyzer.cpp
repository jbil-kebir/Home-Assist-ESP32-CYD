
#include "global.h"
#include "RCSwitchCodeAnalyzer.h"


bool CRCSwitchCodeAnalyzer::decodeHeader(STRUCT_RCS_HEADER& st, unsigned long code) {
    bool ret = true;
    unsigned long *ulptr = (unsigned long*)&st;
    *ulptr = code;
    return ret;
}
bool CRCSwitchCodeAnalyzer::decodeMesure(STRUCT_RCS_MESURE& st, unsigned long code) {
    bool ret = true;
    unsigned long *ulptr = (unsigned long*)&st;
    *ulptr = code;
    return ret;
}
bool CRCSwitchCodeAnalyzer::decodeFooter(STRUCT_RCS_FOOTER& st, unsigned long code) {
    bool ret = true;
    unsigned long *ulptr = (unsigned long*)&st;
    *ulptr = code;
    return ret;
}
void CRCSwitchCodeAnalyzer::printStructRCSMesure(STRUCT_RCS_MESURE stm) {
    DBG(DBG_ACTIONNEURS, "=== MESURE ===\n");
    DBG(DBG_ACTIONNEURS, "ID %d Num %d Type %d Val 0x%x = %d\n", stm.id, stm.numero, stm.type, stm.val, stm.val);

}
void CRCSwitchCodeAnalyzer::printStructSTSMesure(_ST_MESURE_ stm) {
    DBG(DBG_ACTIONNEURS, "=== MESURE ===\n");
    DBG(DBG_ACTIONNEURS, "Type %d Val %.2f\n", stm.type, stm.val);

}
void CRCSwitchCodeAnalyzer::printStructFooter(STRUCT_RCS_FOOTER &stf) {
    DBG(DBG_ACTIONNEURS, "=== FOOTER ===\n");
    DBG(DBG_ACTIONNEURS, "ID %d CRC 0x%x Reserved 0x%x\n", stf.id, stf.crc8, stf.reserved);

}

uint8_t CRCSwitchCodeAnalyzer::computeCRC(const unsigned long* codes, int count) {
  uint8_t crc = 0;
  for (int i = 0; i < count; i++) {
    crc ^= (codes[i] >> 16) & 0xFF;
    crc ^= (codes[i] >> 8) & 0xFF;
    crc ^= codes[i] & 0xFF;
  }
  return crc;
}

bool CRCSwitchCodeAnalyzer::decode(_ST_MESURE_ *st, unsigned long *tab, unsigned char taille) {
    bool ret = true;
    if (st == nullptr || tab == nullptr) return false;

    for (int i = 0; i < taille; i++) {
        STRUCT_RCS_MESURE stmLsb, stmMsb;
        unsigned long ulLsb, ulMsb;
        ulLsb = tab[2*i+1]; ulMsb = tab[2*i];
        
        //Serial.printf("CRCSwitchCodeAnalyzer::decode - ulMsb : 0x%lx ulLsb : 0x%lx\n", ulMsb, ulLsb);

        decodeMesure(stmLsb, ulLsb); //printStructRCSMesure(stmLsb);
        decodeMesure(stmMsb, ulMsb); //printStructRCSMesure(stmMsb);
        unsigned int val = ((unsigned char)stmMsb.val) << 8 | ((unsigned char)stmLsb.val);

        //Serial.printf("CRCSwitchCodeAnalyzer::decode - val : 0x%lx = %ld\n", val, val);
        
        st[i].type = (int)stmLsb.type;
        switch(st[i].type) {
            case 0: // Température 
            st[i].val = ((float)val / 10.0) - 20.0;
            DBG(DBG_ACTIONNEURS, "CRCSwitchCodeAnalyzer::decode () - Mesure Température %.1f\n", st[i].val);
            break;
            case 1: // Humidité 
            st[i].val = (float)val;
            DBG(DBG_ACTIONNEURS, "CRCSwitchCodeAnalyzer::decode () - Mesure Humidité %.0f\n", st[i].val);
            break;
            default: // Inconnue
            DBG(DBG_ACTIONNEURS, "CRCSwitchCodeAnalyzer::decode () - Mesure %d : Type inconnu %d, valLsb %d, valMsb %d\n", stmLsb.numero, stmLsb.type, stmLsb.val, stmMsb.val);
            break;
        }
        
    }
    return true;
}
