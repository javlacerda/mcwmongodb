/*
** Utility functions
*/

#include "Utils.h"
#include <memory>

/*
** Gets a std::wstring from an EXTCompInfo structure
*/
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, std::wstring &destValue, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramVal(reinterpret_cast<qfldval>(param->mData));
   if (paramVal.isNull())
      return allowNull;

   std::unique_ptr<qchar[]> textTmp(new qchar[paramVal.getCharLen() + sizeof(qchar)]);
   qlong lenTransf;

   paramVal.getChar(paramVal.getCharLen(), textTmp.get(), lenTransf);
   textTmp[paramVal.getCharLen()] = 0;

   CHRconvToOs cConv(textTmp.get());
   destValue = reinterpret_cast<wchar_t *>(cConv.dataPtr());
   return true;
}

/*
** Gets a std::string from an EXTCompInfo structure
*/
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, std::string &destValue, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (param == nullptr)
      return false;

   EXTfldval paramVal(reinterpret_cast<qfldval>(param->mData));
   if (paramVal.isNull())
      return allowNull;

   std::unique_ptr<qchar[]> textTmp(new qchar[paramVal.getCharLen() + sizeof(qchar)]);
   qlong lenTransf;

   paramVal.getChar(paramVal.getCharLen(), textTmp.get(), lenTransf);
   textTmp[paramVal.getCharLen()] = 0;

   CHRconvToBytes converter(textTmp.get());
   destValue = reinterpret_cast<char *>(converter.dataPtr());
   return true;
}

/*
** Gets a str255 from an EXTCompInfo structure
*/
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, str255 &destValue, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramVal(reinterpret_cast<qfldval>(param->mData));
   if (paramVal.isNull())
      return allowNull;

   destValue = paramVal.getChar();
   return true;
}

/*
** Gets a std::vector<qchar> from an EXTCompInfo structure
*/
bool getStringFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, std::vector<qchar> &destValue, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (param == nullptr)
      return false;

   EXTfldval paramVal(reinterpret_cast<qfldval>(param->mData));
   if (paramVal.isNull())
      return allowNull;

   destValue.reserve(paramVal.getCharLen() + 1);
   qlong lenTransf;

   paramVal.getChar(paramVal.getCharLen(), destValue.data(), lenTransf);
   destValue[paramVal.getCharLen()] = 0;
   return true;
}

/*
** Gets a EXTqlist class from an EXTCompInfo structure
*/
EXTqlist *getListFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, bool duplicate)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return nullptr;

   EXTfldval fval(reinterpret_cast<qfldval>(param->mData));

   ffttype fvalType;
   fval.getType(fvalType);
   if (fvalType != fftList)
      return nullptr;

   return fval.getList(duplicate ? qtrue : qfalse);
}

/*
** Gets a EXTqlist class from an EXTCompInfo structure.
*/
EXTqlist *getListFromEXTqlist(qlong rowno, qshort columnno, EXTqlist *list, bool duplicate)
{
   EXTfldval fldValue;
   list->getColValRef(rowno, columnno, fldValue, qfalse);

   if (fldValue.isNull())
      return nullptr;

   ffttype fvalType;
   fldValue.getType(fvalType);
   if (fvalType != fftList)
      return nullptr;

   return fldValue.getList(duplicate ? qtrue : qfalse);
}

/*
** Gets a std::string from an EXTqlist row/col
*/
bool getStringFromEXTqlist(qlong rowno, qshort columnno, EXTqlist *list, std::string &destValue)
{
   EXTfldval fldValue;
   list->getColVal(rowno, columnno, fldValue);
   return getStringFromEXTfldval(fldValue, destValue);
}

/*
** Gets a std::string from an EXTfldval structure
*/
bool getStringFromEXTfldval(EXTfldval &fval, std::string &destValue)
{
   qlong charLen = fval.getCharLen();

   std::unique_ptr<qchar[]> textTmp(new qchar[charLen + sizeof(qchar)]);
   qlong lenTransf;

   fval.getChar(charLen, textTmp.get(), lenTransf);
   textTmp[charLen] = 0;

   CHRconvToBytes converter(textTmp.get());
   destValue = reinterpret_cast<char *>(converter.dataPtr());
   return true;
}

/*
** Gets a std::string from an EXTqlist row/col
*/
bool getStringFromEXTqlist(qlong rowno, qshort columnno, EXTqlist &list, std::string &destValue)
{
   EXTfldval fldValue;
   list.getColVal(rowno, columnno, fldValue);
   return getStringFromEXTfldval(fldValue, destValue);
}

/*
** Sets a std::string to an EXTfldval structure
*/
void stringToEXTfldval(const std::string &text, EXTfldval &result, const qshort uniType)
{
   if (uniType == preUniTypeAuto) {
      CHRconvFromBytes cConv(reinterpret_cast<qbyte *>(const_cast<char *>(text.c_str())), static_cast<qlong>(text.size()));
      result.setChar(cConv.dataPtr(), cConv.len());
   }
   else {
      CHRconvFromCodePage cConv(uniType, reinterpret_cast<qbyte *>(const_cast<char *>(text.c_str())), static_cast<qlong>(text.size()));
      result.setChar(cConv.dataPtr(), cConv.len());
   }
}

/*
** Sets a std::wstring to an EXTfldval structure
*/
void stringToEXTfldval(const std::wstring &text, EXTfldval &result)
{
   CHRconvFromOs cConv(reinterpret_cast<qoschar *>(const_cast<wchar_t *>(text.c_str())));
   result.setChar(cConv.dataPtr(), cConv.len());
}

/*
** Sets a std::string to an EXTqlist structure
*/
void stringToEXTqlist(qlong rowno, qshort columnno, const std::string &value, EXTqlist *destList, const qshort uniType)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);

   if (uniType == preUniTypeAuto) {
      CHRconvFromBytes cConv(reinterpret_cast<qbyte *>(const_cast<char *>(value.c_str())), static_cast<qlong>(value.size()));
      fldvalCol.setChar(cConv.dataPtr(), cConv.len());
   }
   else {
      CHRconvFromCodePage cConv(uniType, reinterpret_cast<qbyte *>(const_cast<char *>(value.c_str())), static_cast<qlong>(value.size()));
      fldvalCol.setChar(cConv.dataPtr(), cConv.len());
   }
}

/*
** Sets a std::wstring to an EXTqlist structure
*/
void stringToEXTqlist(qlong rowno, qshort columnno, const std::wstring &value, EXTqlist *destList)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   stringToEXTfldval(value, fldvalCol);
}

/*
** Sets a std::wstring to an EXTCompInfo structure
*/
bool stringToEXTCompInfo(const std::wstring &orig, EXTCompInfo *pEci, const qshort paramID)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramTexto(reinterpret_cast<qfldval>(param->mData));

   CHRconvFromOs cConv(reinterpret_cast<qoschar *>(const_cast<wchar_t *>(orig.c_str())));
   paramTexto.setChar(cConv.dataPtr(), cConv.len());

   ECOsetParameterChanged(pEci, paramID);
   return true;
}

/*
** Sets a EXTqlist to an EXTqlist structure
*/
void listToEXTqlist(qlong rowno, qshort columnno, EXTqlist *value, EXTqlist *destList, bool transferOwnership)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setList(value, transferOwnership ? qtrue : qfalse);
}

/*
** Gets an integer from an EXTCompInfo structure
*/
bool getIntFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, int &destValue, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramValue(reinterpret_cast<qfldval>(param->mData));
   if (paramValue.isNull() && !allowNull)
      return false;

   destValue = static_cast<int>(paramValue.getLong());
   return true;
}

/*
** Gets an integer from an EXTfldval structure
*/
bool getIntFromEXTfldval(EXTfldval &fldValue, int &destValue, bool allowNull)
{
   if (fldValue.isNull())
      return allowNull;

   destValue = static_cast<int>(fldValue.getLong());
   return true;
}

/*
** Gets an integer from an EXTqlist row/col
*/
bool getIntFromEXTqlist(qlong rowno, qshort columnno, EXTqlist *list, int &destValue, bool allowNull)
{
   EXTfldval fldValue;
   list->getColVal(rowno, columnno, fldValue);
   return getIntFromEXTfldval(fldValue, destValue, allowNull);
}

/*
** Gets a boolean from an EXTCompInfo
*/
bool getBoolFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, bool &destValue, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval fldValue(reinterpret_cast<qfldval>(param->mData));
   if (fldValue.isNull())
      return allowNull;

   destValue = fldValue.getBool() == preBoolTrue;
   return true;
}

/*
** Gets a boolean from an EXTfldval structure
*/
bool getBoolFromEXTfldval(EXTfldval &fldValue, int &destValue, bool allowNull)
{
   if (fldValue.isNull())
      return allowNull;

   destValue = static_cast<int>(fldValue.getLong());
   return true;
}

/*
** Ler um parâmetro do tipo double a partir de uma estrutura EXTCompInfo
*/
bool getDoubleFromEXTCompInfo(EXTCompInfo *pEci, qlong paramID, double &valor, bool allowNull)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval fldValue(reinterpret_cast<qfldval>(param->mData));
   if (fldValue.isNull())
      return allowNull;

   qshort dp = dpDefault;
   fldValue.getNum(valor, dp);
   return true;
}

/*
** Ler um parâmetro do tipo double a partir de uma estrutura EXTCompInfo
*/
bool getDoubleFromEXTfldval(EXTfldval &fldValue, double &valor, bool allowNull)
{
   if (fldValue.isNull())
      return allowNull;

   qshort dp = dpDefault;
   fldValue.getNum(valor, dp);
   return true;
}

/*
** Copiar uma variável do tipo boolean para a estrutura EXTCompInfo
*/
bool boolToEXTCompInfo(const bool valor, EXTCompInfo *pEci, const qshort paramID)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramValor(reinterpret_cast<qfldval>(param->mData));
   paramValor.setBool(valor ? preBoolTrue : preBoolFalse);
   ECOsetParameterChanged(pEci, paramID);
   return qtrue;
}

/*
** Sets a bool value to an EXTfldval structure
*/
void boolToEXTfldval(const bool value, EXTfldval &result)
{
   result.setBool(value ? preBoolTrue : preBoolFalse);
}

/*
** Sets a qbool value to an EXTfldval structure
*/
void boolToEXTfldval(const qbool value, EXTfldval &result)
{
   result.setBool((value == qtrue) ? preBoolTrue : preBoolFalse);
}

/*
** Sets a bool value to an EXTqlist row/col
*/
void boolToEXTqlist(qlong rowno, qshort columnno, const bool value, EXTqlist &list)
{
   EXTfldval fldvalCol;
   list.getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setBool(value ? preBoolTrue : preBoolFalse);
}

void boolToEXTqlist(qlong rowno, qshort columnno, const bool value, EXTqlist *destList)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setBool(value ? preBoolTrue : preBoolFalse);
}

/*
** Sets an int value to an EXTfldval structure
*/
void intToEXTfldval(const int32_t value, EXTfldval &result)
{
   result.setLong(value);
}

/*
** Sets an int value to an EXTqlist row/col
*/
void intToEXTqlist(qlong rowno, qshort columnno, const int32_t value, EXTqlist &list)
{
   EXTfldval fldvalCol;
   list.getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setLong(value);
}

void intToEXTqlist(qlong rowno, qshort columnno, const int32_t value, EXTqlist *destList)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setLong(value);
}

/*
** Sets an int value to an EXTfldval structure
*/
bool intToEXTCompInfo(const int32_t valor, EXTCompInfo *pEci, const qshort paramID)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramValor(reinterpret_cast<qfldval>(param->mData));
   paramValor.setLong(valor);
   ECOsetParameterChanged(pEci, paramID);
   return true;
}

/*
** Sets an int64_t value to an EXTfldval
*/
void longToEXTfldval(const int64_t value, EXTfldval &result)
{
   result.setLong64(value);
}

/*
** Sets an int64_t value to an EXTqlist row/col
*/
void longToEXTqlist(qlong rowno, qshort columnno, const int64_t value, EXTqlist &list)
{
   EXTfldval fldvalCol;
   list.getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setLong64(value);
}

/*
** Sets an int64_t value to an EXTqlist row/col
*/
void longToEXTqlist(qlong rowno, qshort columnno, const int64_t value, EXTqlist *destList)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setLong64(value);
}

/*
** Sets a double value to an EXTqlist row/col
*/
void doubleToEXTqlist(qlong rowno, qshort columnno, const double value, EXTqlist *destList)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setNum(value);
}

/*
** Sets a null value to an EXTqlist row/col
*/
void nullToEXTqlist(qlong rowno, qshort columnno, EXTqlist *destList)
{
   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setNull();
}

/*
** Sets a DateTime value to an EXTqlist row/col
*/
void dateTimeToEXTqlist(qlong rowno, qshort columnno, const std::chrono::milliseconds value, EXTqlist *destList)
{
   std::chrono::system_clock::time_point tp{ value };
   std::time_t t = std::chrono::system_clock::to_time_t(tp);
   std::tm tm{};
#if defined(_WIN32)
   gmtime_s(&tm, &t);
#else
   gmtime_r(&t, &tm);
#endif

   datestamptype omnisDateTime{};
   omnisDateTime.mYear = static_cast<qshort>(tm.tm_year + 1900);
   omnisDateTime.mMonth = static_cast<char>(tm.tm_mon + 1);
   omnisDateTime.mDay = static_cast<char>(tm.tm_mday);
   omnisDateTime.mDateOk = static_cast<char>(qtrue);

   omnisDateTime.mHour = static_cast<char>(tm.tm_hour);
   omnisDateTime.mMin = static_cast<char>(tm.tm_min);
   omnisDateTime.mSec = static_cast<char>(tm.tm_sec);
   omnisDateTime.mSecOk = static_cast<char>(qtrue);
   omnisDateTime.mTimeOk = static_cast<char>(qtrue);
   omnisDateTime.mHunOk = static_cast<char>(qfalse);

   EXTfldval fldvalCol;
   destList->getColValRef(rowno, columnno, fldvalCol, qtrue);
   fldvalCol.setDate(omnisDateTime);
}

/*
** Sets a long value to an EXTfldval structure
*/
bool longToEXTCompInfo(const int64_t valor, EXTCompInfo *pEci, const qshort paramID)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramValor(reinterpret_cast<qfldval>(param->mData));
   paramValor.setLong64(valor);
   ECOsetParameterChanged(pEci, paramID);
   return true;
}

/*
** Sets a double value to an EXTfldval structure
*/
bool doubleToEXTCompInfo(const double valor, EXTCompInfo *pEci, const qshort paramID)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramValor(reinterpret_cast<qfldval>(param->mData));
   paramValor.setNum(valor);
   ECOsetParameterChanged(pEci, paramID);
   return true;
}

/*
** Converts a char array to a std::wstring
*/
bool charToWstring(const char *source, std::wstring &destination)
{
   size_t sourceSize = strlen(source);

   std::unique_ptr<wchar_t[]> buffer(new wchar_t[sourceSize + 1]);
   size_t rc = std::mbstowcs(buffer.get(), source, sourceSize + 1);
   if (rc != -1)
      destination = buffer.get();
   else
      destination = L"charToWstring: Conversion Error!!";

   return rc != -1;
}

/*
** Converts a std::string to a std::wstring
*/
bool stringToWstring(const std::string &source, std::wstring &destination)
{
   return charToWstring(source.c_str(), destination);
}

/*
** Sets a binary to a EXTCompInfo class
*/
bool binaryToEXTCompInfo(EXTCompInfo *pEci, const qshort paramID, unsigned char *value, int size)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramTexto(reinterpret_cast<qfldval>(param->mData));
   paramTexto.setBinary(fftBinary, value, size);

   ECOsetParameterChanged(pEci, paramID);
   return true;
}

/*
** Sets a binary to a EXTCompInfo class
* */
bool binaryToEXTCompInfo(EXTCompInfo *pEci, const qshort paramID, char *value, int size)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramTexto(reinterpret_cast<qfldval>(param->mData));
   paramTexto.setBinary(fftBinary, reinterpret_cast<unsigned char *>(value), size);

   ECOsetParameterChanged(pEci, paramID);
   return true;
}

/*
** Adds a fftCharacter/dpFcharacter column to an EXTqlist object
*/
qshort addCharacterColToEXTqlist(EXTqlist *destList, const qlong fln, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftCharacter, dpFcharacter, fln, &colName);
}

/*
** Adds an fftInteger column no an EXTqlist object
*/
qshort addIntegerColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftInteger, 0, 0, &colName);
}

/*
** Adds an fftInteger/dpF64bitinteger column no an EXTqlist object
*/
qshort addInteger64ColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftInteger, dpF64bitinteger, 0, &colName);
}

/*
** Adds an fftInteger/dpF64bitinteger column no an EXTqlist object
*/
qshort addDoubleColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftNumber, dpFloat, 0, &colName);
}

/*
** Adds an fftBoolean column no an EXTqlist object
*/
qshort addBoolColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftBoolean, 0, 0, &colName);
}

/*
** Adds an EXTqlist column no an EXTqlist object
*/
qshort addEXTqlistColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftList, 0, 0, &colName);
}

/*
** Adds a Row column no an EXTqlist object
*/
qshort addRowColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftList, dpFrow, 0, &colName);
}

/*
** Adds a Datetime column no an EXTqlist object
*/
qshort addDateTimeColToEXTqlist(EXTqlist *destList, const char *name)
{
   str255 colName(name);
   return destList->addCol(fftDate, dpFdtimeC, 0, &colName);
}

/*
** Converts an array of chars to str255 object
*/
void charToStr255(const char *text, str255 &omnisString)
{
   omnisString.setUtf8(reinterpret_cast<qbyte *>(const_cast<char *>(text)), static_cast<qlong>(strlen(text)));
}

str255 charToStr255(const char *text)
{
   CHRconvFromBytes cConv(reinterpret_cast<qbyte *>(const_cast<char *>(text)));
   str255 omnisString(cConv.dataPtr());
   return omnisString;
}

/*
** Replaces Omnis New LIne chars with cpp standard new line
*/
void replaceOmnisNewLine(std::wstring &text)
{
   if (text.empty())
      return;

   std::wstring::size_type pos = 0;
   const std::wstring omnisNewLine(L"//");
   const std::wstring newLine(L"\n");

   while ((pos = text.find(omnisNewLine, pos)) != std::wstring::npos) {
      text.replace(pos, omnisNewLine.size(), newLine);
      pos += newLine.length();
   }
}

/*
** Send information to Omnis trace log
*/
void sendToOmnisTraceLog(const char *msg)
{
   CHRconvFromBytes conv(reinterpret_cast<qbyte *>(const_cast<char *>(msg)));
   str255 msgStr(conv.dataPtr());
   ECOaddTraceLine(&msgStr);
}

void sendToOmnisTraceLog(const char *msg1, const char *msg2)
{
   CHRconvFromBytes conv1(reinterpret_cast<qbyte *>(const_cast<char *>(msg1)));
   str255 msgStr1(conv1.dataPtr());

   CHRconvFromBytes conv2(reinterpret_cast<qbyte *>(const_cast<char *>(msg2)));
   str255 msgStr2(conv2.dataPtr());

   ECOaddTraceLine(&msgStr1, &msgStr2);
}

void sendToOmnisTraceLog(const char *msg1, const str255 &msg2)
{
   CHRconvFromBytes conv1(reinterpret_cast<qbyte *>(const_cast<char *>(msg1)));
   str255 msgStr1(conv1.dataPtr());
   ECOaddTraceLine(&msgStr1, const_cast<str255 *>(&msg2));
}

void sendToOmnisTraceLog(const char *msg1, const wchar_t *msg2)
{
   CHRconvFromBytes conv1(reinterpret_cast<qbyte *>(const_cast<char *>(msg1)));
   str255 msgStr1(conv1.dataPtr());
   str255 msgStr2(msg2);
   ECOaddTraceLine(&msgStr1, &msgStr2);
}

/*
** Checks if one param exists
*/
bool checkParamExists(EXTCompInfo *pEci, qlong paramID)
{
   EXTParamInfo *param = ECOfindParamNum(pEci, paramID);
   if (!param)
      return false;

   EXTfldval paramVal(reinterpret_cast<qfldval>(param->mData));
   return (paramVal.isNull() != qtrue);
}
