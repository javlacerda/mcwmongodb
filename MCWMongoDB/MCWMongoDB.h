#ifndef _MCW_MONGO_DB_H
#define _MCW_MONGO_DB_H

#include <mongoc/mongoc.h>
#include <extcomp.he>
#include <string>
#include <vector>

const qshort LIB_RES_NAME = 1000;
const qlong RES_CONST_PREFIX = 19999;
const qlong RES_CONST_START = 20000;
const qlong RES_CONST_END = 20012;

const qshort VERSION_MAJOR = 1;
const qshort VERSION_MINOR = 0;

const qshort cObject_MongoDB = 1;

const qlong cSetConnectionStringFunction = 1;
const qlong cSetDatabaseFunction = 2;
const qlong cSetCollectionFunction = 3;
const qlong cGetErrorMessageFunction = 4;
const qlong cFindFunction = 5;
const qlong cAddFilterFunction = 6;
const qlong cSetDynamicFilterFunction = 7;
const qlong cClearFiltersFunction = 8;

enum FilterOperators {
   foEQUAL = 0,
   foNOT_EQUAL = 1,
   foGREATER = 2,
   foGREATER_OR_EQUAL = 3,
   foLESSER = 4,
   foLESSER_OR_EQUAL = 5,
   foCOUNT = 6
};

enum DataTypes {
   dtCHARACTER = 0,
   dtINTEGER_32 = 1,
   dtINTEGER_64 = 2,
   dtDOUBLE = 3,
   dtBOOLEAN = 4,
   dtDATE_TIME = 5,
   dtNULL = 6,
   dtCOUNT = 7
};

struct FilterItem {
   std::string fieldName;
   std::string value;
   FilterOperators oper;
   DataTypes type;
};

class CMongoDB
{
private:
   std::string m_errorMessage;
   std::string m_connectionString;
   std::string m_database;
   std::string m_collection;
   std::vector<FilterItem> m_filters;
   std::string m_dynamicFilter;

   // mongoc handles (nullptr when not connected)
   mongoc_client_t *m_Client;
   mongoc_database_t *m_Database;
   mongoc_collection_t *m_Collection;

public:
   qobjinst m_ObjPtr;

public:
   CMongoDB(qobjinst pObjPtr);
   CMongoDB(qobjinst pObjPtr, const CMongoDB *src);
   ~CMongoDB();
   void setObject(qobjinst pObjPtr, const CMongoDB *src);

   qbool methodCall(EXTCompInfo *pEci);
   qbool setConnectionString(EXTCompInfo *pEci);
   qbool setDatabase(EXTCompInfo *pEci);
   qbool setCollection(EXTCompInfo *pEci);
   void getErrorMessage(EXTCompInfo *pEci) const;
   qbool find(EXTCompInfo *pEci);
   qbool setDynamicFilter(EXTCompInfo *pEci);
   qbool addFilter(EXTCompInfo *pEci);
   qbool clearFilters(EXTCompInfo *pEci);

private:
   void releaseHandles();
   bool openCollection();

   std::string getElementValue(const bson_iter_t &iter);
   DataTypes getElementType(const bson_iter_t &iter);
   bool validateFind();
   bson_t *buildFilter();
   bool buildFilterItem(bson_t *dest, const std::string &fieldName, FilterOperators filterOperator, const std::string &value, DataTypes type);
   bool appendBsonValue(bson_t *doc, const std::string &key, const std::string &value, DataTypes type);
   bool iso8601ToMilliseconds(const std::string &iso, int64_t &retMs);
   void fillDestList(const bson_t *doc, EXTqlist *rowList);
};

#endif
