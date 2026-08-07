// Prozis.Tech, S. A.
// Utility functions

#ifndef _UTILS_H_
#define _UTILS_H_

#include <extcomp.he>
#include <dmconst.he>
#include <chrono>
#include <string>

bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, std::wstring &destValue, bool allowNull = false);
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, std::string &destValue, bool allowNull = false);
bool getStringFromEXTfldval(EXTfldval &valor, std::string &destino);
bool getStringFromEXTqlist(qlong rowno, qshort columnno, EXTqlist &list, std::string &destValue);
bool getStringFromEXTqlist(qlong rowno, qshort columnno, EXTqlist *list, std::string &destValue);
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, str255 &destValue, bool allowNull = false);
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, std::vector<qchar> &destValue, bool allowNull = false);

EXTqlist *getListFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, bool duplicate);
EXTqlist *getListFromEXTqlist(qlong rowno, qshort columnno, EXTqlist *list, bool duplicate);

bool getIntFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, int &valor, bool allowNull = false);
bool getIntFromEXTfldval(EXTfldval &fldValue, int &destValue, bool allowNull = false);
bool getIntFromEXTqlist(qlong rowno, qshort columnno, EXTqlist *list, int &destValue, bool allowNull = false);

bool getInt64FromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, int64_t &valor, bool allowNull = false);

bool getDoubleFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, double &valor, bool allowNull = false);
bool getDoubleFromEXTfldval(EXTfldval &fldValue, double &valor, bool allowNull = false);

bool getBoolFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, bool &destValue, bool allowNull = false);
bool getBoolFromEXTfldval(EXTfldval &fldValue, int &destValue, bool allowNull);

bool getEXTfldvalFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, EXTfldval &fldval);

bool getParamType(EXTCompInfo *pEci, qlong paramID, ffttype &paramType, qshort *paramSubtype = nullptr);

void stringToEXTfldval(const std::string &text, EXTfldval &result, const qshort uniType = preUniTypeAuto);
void stringToEXTfldval(const std::wstring &text, EXTfldval &result);
void stringToEXTqlist(qlong rowno, qshort columnno, const std::string &value, EXTqlist *destList, const qshort uniType = preUniTypeAuto);
void stringToEXTqlist(qlong rowno, qshort columnno, const std::wstring &value, EXTqlist *destList);
bool stringToEXTCompInfo(const std::wstring &orig, EXTCompInfo *pEci, const qshort paramID);

void listToEXTqlist(qlong rowno, qshort columnno, EXTqlist *value, EXTqlist *destList, bool transferOwnership);

bool boolToEXTCompInfo(const bool valor, EXTCompInfo *pEci, const qshort paramID);
void boolToEXTfldval(const bool value, EXTfldval &result);
void boolToEXTfldval(const qbool value, EXTfldval &result);
void boolToEXTqlist(qlong rowno, qshort columnno, const bool value, EXTqlist &list);
void boolToEXTqlist(qlong rowno, qshort columnno, const bool value, EXTqlist *destList);

void longToEXTfldval(const int64_t value, EXTfldval &result);
void longToEXTqlist(qlong rowno, qshort columnno, const int64_t value, EXTqlist &list);
void longToEXTqlist(qlong rowno, qshort columnno, const int64_t value, EXTqlist *destList);
bool longToEXTCompInfo(const int64_t valor, EXTCompInfo *pEci, const qshort paramID);

void intToEXTfldval(const int32_t value, EXTfldval &result);
void intToEXTqlist(qlong rowno, qshort columnno, const int32_t value, EXTqlist &list);
void intToEXTqlist(qlong rowno, qshort columnno, const int32_t value, EXTqlist *destList);
bool intToEXTCompInfo(const int32_t valor, EXTCompInfo *pEci, const qshort paramID);

bool doubleToEXTCompInfo(const double valor, EXTCompInfo *pEci, const qshort paramID);
void doubleToEXTqlist(qlong rowno, qshort columnno, const double value, EXTqlist *destList);

bool binaryToEXTCompInfo(EXTCompInfo *pEci, const qshort paramID, unsigned char *orig, int tamanho);
bool binaryToEXTCompInfo(EXTCompInfo *pEci, const qshort paramID, char *value, int size);

void dateTimeToEXTqlist(qlong rowno, qshort columnno, const std::chrono::milliseconds value, EXTqlist *destList);

void nullToEXTqlist(qlong rowno, qshort columnno, EXTqlist *destList);

bool charToWstring(const char *source, std::wstring &destination);
bool stringToWstring(const std::string &source, std::wstring &destination);
void charToStr255(const char *text, str255 &omnisString);
str255 charToStr255(const char *text);
void replaceOmnisNewLine(std::wstring &text);

qshort addCharacterColToEXTqlist(EXTqlist *destList, const qlong fln, const char *name);
qshort addIntegerColToEXTqlist(EXTqlist *destList, const char *name);
qshort addInteger64ColToEXTqlist(EXTqlist *destList, const char *name);
qshort addBoolColToEXTqlist(EXTqlist *destList, const char *name);
qshort addEXTqlistColToEXTqlist(EXTqlist *destList, const char *name);
qshort addRowColToEXTqlist(EXTqlist *destList, const char *name);
qshort addDoubleColToEXTqlist(EXTqlist *destList, const char *name);
qshort addDateTimeColToEXTqlist(EXTqlist *destList, const char *name);

void sendToOmnisTraceLog(const char *msg);
void sendToOmnisTraceLog(const char *msg1, const char *msg2);
void sendToOmnisTraceLog(const char *msg1, const wchar_t *msg2);
void sendToOmnisTraceLog(const char *msg1, const str255 &msg2);

bool checkParamExists(EXTCompInfo *pEci, qlong paramID);
#endif
