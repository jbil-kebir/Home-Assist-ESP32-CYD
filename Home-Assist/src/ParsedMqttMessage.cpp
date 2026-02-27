#include "ParsedMqttMessage.h"

// Méthodes de détection (à mettre dans la classe ou en helpers)
bool ParsedMqttMessage::isForce(const String& s) const {
    return s.equalsIgnoreCase("FORCE");
}

bool ParsedMqttMessage::isIp(const String& s) const {
    if (s.length() < 7) return false;  // minimum 0.0.0.0

    int dotCount = 0;
    bool valid = true;
    for (char c : s) {
        if (c == '.') dotCount++;
        else if (!isDigit(c)) valid = false;
    }

    return valid && dotCount == 3;
}

bool ParsedMqttMessage::isDate(const String& s) const {
    // Format très simple : contient '/', longueur raisonnable
    return s.indexOf('/') >= 0 && s.length() >= 8 && s.length() <= 10;
}

bool ParsedMqttMessage::isTime(const String& s) const {
    // Format hh:mm:ss, contient ':'
    return s.indexOf(':') >= 0 && s.length() >= 6 && s.length() <= 8;
}

bool ParsedMqttMessage::parse(const String& payload) {
    mbIsValid = false;
    msExpediteur.clear();
    mvsMesure.clear();
    miTailleMesure = 0;
    msDate.clear();
    msTime.clear();
    msIp.clear();
    mfValeur = 0.0f;
    mbForce = false;

    String s = payload;
    s.trim();

    if (s.length() == 0) return false;

    std::vector<String> tokens;
    
// ──────────────────────────────────────────────────────────────
// Découpage propre en tokens (remplace le commentaire)
// ──────────────────────────────────────────────────────────────
int pos = 0;
while (pos < s.length()) {
    // Sauter tous les espaces consécutifs
    while (pos < s.length() && s[pos] == ' ') {
        pos++;
    }
    if (pos >= s.length()) break;

    // Trouver la fin du token (prochain espace ou fin de chaîne)
    int end = pos;
    while (end < s.length() && s[end] != ' ') {
        end++;
    }

    // Extraire le token et le nettoyer (trim)
    String token = s.substring(pos, end);
    token.trim();

    // Ajouter seulement si non vide
    if (token.length() > 0) {
        tokens.push_back(token);
    }

    // Avancer au prochain token
    pos = end;
}

// ──────────────────────────────────────────────────────────────
// Suite du traitement (tu pourras l'améliorer plus tard)
// ──────────────────────────────────────────────────────────────

if (tokens.size() < 2) {  // au moins expéditeur + 1 champ
    return false;
}

// 1. Le premier token est toujours l'expéditeur
msExpediteur = tokens[0];

for (size_t i = 1; i < tokens.size(); ++i) {
    String current = tokens[i];

    if (isForce(current)) {
        mbForce = true;
        // Ne pas ajouter dans mvsMesure
    }
    else if (isIp(current)) {
        msIp = current;
        // Ne pas ajouter dans mvsMesure
    }
    else if (isDate(current)) {
        msDate = current;
        // Ne pas ajouter dans mvsMesure
    }
    else if (isTime(current)) {
        msTime = current;
        // Ne pas ajouter dans mvsMesure
    }
    else {
        // Tout le reste → mesure
        mvsMesure.push_back(current);
    }
}

miTailleMesure = mvsMesure.size();   
mbIsValid = (tokens.size() > 0);
return mbIsValid;
}

/*bool ParsedMqttMessage::parse(const String& payload) {
    mbIsValid = false;
    msExpediteur.clear();
    mvsMesure.clear();
    miTailleMesure = 0;
    msDate.clear();
    msTime.clear();
    msIp.clear();
    mfValeur = 0.0f;
    mbForce = false;

    String s = payload;
    s.trim();

    if (s.length() == 0) return false;

    std::vector<String> tokens;
    int pos = 0;
    while (pos < s.length()) {
        while (pos < s.length() && s[pos] == ' ') pos++;
        if (pos >= s.length()) break;

        int next = s.indexOf(' ', pos);
        if (next == -1) next = s.length();

        String token = s.substring(pos, next);
        token.trim();
        if (token.length() > 0) tokens.push_back(token);

        pos = next;
    }

    if (tokens.size() < 4) return false;

    // 1. Expéditeur = premier token
    msExpediteur = tokens[0];

    // 2. IP = dernier token (IPv4)
    String last = tokens.back();
    if (last.indexOf('.') > 0) {
        int dotCount = 0;
        bool validIp = true;
        for (char c : last) {
            if (c == '.') dotCount++;
            else if (!isDigit(c)) validIp = false;
        }
        if (validIp && dotCount == 3) {
            msIp = last;
            tokens.pop_back();
        }
    }

    // 3. FORCE = juste avant IP (ou à la fin si pas d'IP)
    if (!tokens.empty() && tokens.back().equalsIgnoreCase("FORCE")) {
        mbForce = true;
        tokens.pop_back();
    }

    // 4. Date et heure = les deux derniers tokens (avant FORCE/IP)
    int n = tokens.size();
    if (n >= 4) {
        String candidateTime = tokens[n-1];
        String candidateDate = tokens[n-2];

        // Détection date : contient '/', longueur >=8
        // Time : contient ':', longueur >=6
        if (candidateDate.indexOf('/') >= 0 && candidateDate.length() >= 8 &&
            candidateTime.indexOf(':') >= 0 && candidateTime.length() >= 6) {
            msDate = candidateDate;
            msTime = candidateTime;
            n -= 2;
        }
    }

    // 5. Le reste (après expéditeur, avant date/time) = mesures + valeur numérique
    for (int i = 1; i < n; ++i) {
        mvsMesure.push_back(tokens[i]);
    }

    // 6. Extraire la valeur numérique (généralement le dernier champ de mesures)
    if (!mvsMesure.empty()) {
        String lastMesure = mvsMesure.back();
        if (lastMesure.indexOf('.') >= 0 || isDigit(lastMesure[0]) || (lastMesure[0] == '-' && lastMesure.length() > 1)) {
            mfValeur = lastMesure.toFloat();
            mvsMesure.pop_back();  // retire la valeur
        }
    }

    miTailleMesure = mvsMesure.size();

    // Validation finale
    mbIsValid = (miTailleMesure >= 1 && msIp.length() > 0);
    return mbIsValid;
}*/

void ParsedMqttMessage::printDebug() const {
    if (!mbIsValid) {
        Serial.println("Message MQTT invalide");
        return;
    }

    Serial.println("Parsed MQTT message:");
    Serial.print("  Expéditeur    : "); Serial.println(msExpediteur);
    Serial.print("  Mesures       : [");
    for (size_t i = 0; i < mvsMesure.size(); ++i) {
        if (i > 0) Serial.print(", ");
        Serial.print(mvsMesure[i]);
    }
    Serial.println("]");
    Serial.print("  Taille mesures: "); Serial.println(miTailleMesure);
    if (msDate.length() > 0) {
        Serial.print("  Date          : "); Serial.println(msDate);
    }
    if (msTime.length() > 0) {
        Serial.print("  Heure         : "); Serial.println(msTime);
    }
    if (mfValeur != 0.0f) {
        Serial.print("  Valeur        : "); Serial.println(mfValeur, 1);
    }
    if (msIp.length() > 0) {
        Serial.print("  IP            : "); Serial.println(msIp);
    }
    Serial.print("  Force         : "); Serial.println(mbForce ? "OUI" : "NON");
    Serial.println();
}