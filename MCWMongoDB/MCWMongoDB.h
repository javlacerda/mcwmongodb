#ifndef _MCW_MONGO_DB_H
#define _MCW_MONGO_DB_H

#include <mongoc/mongoc.h>
#include <extcomp.he>
#include <string>

const qshort LIB_RES_NAME = 1000;
const qlong RES_CONST_PREFIX = 19999;
const qlong RES_CONST_START = 20006;
const qlong RES_CONST_END = 20012;

const qshort VERSION_MAJOR = 1;
const qshort VERSION_MINOR = 9;

const qshort cObject_MongoDB = 1;

const qlong cSetConnectionStringFunction = 1;
const qlong cSetDatabaseFunction = 2;
const qlong cSetCollectionFunction = 3;
const qlong cGetErrorMessageFunction = 4;
const qlong cFindFunction = 5;
const qlong cSetFilterFunction = 7;
const qlong cSetOptsFunction = 10;
const qlong cSetAggregateFunction = 11;
const qlong cAggregateFunction = 12;

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

class CMongoDB
{
private:
   std::string m_errorMessage;
   std::string m_connectionString;
   std::string m_databaseName;
   std::string m_collectionName;
   std::string m_filter;
   std::string m_opts;
   std::string m_aggregatePipeline;

   // mongoc pool (nullptr when not connected).
   // A pool is used instead of a plain mongoc_client_t so that server
   // discovery for replica sets runs in background monitoring threads
   // (mongoc_client_pool_pop) instead of blocking synchronously on every
   // seed host in turn on the calling thread.
   mongoc_client_pool_t *m_Pool;

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
   qbool setFilter(EXTCompInfo *pEci);
   qbool setOpts(EXTCompInfo *pEci);
   qbool setAggregate(EXTCompInfo *pEci);
   qbool aggregate(EXTCompInfo *pEci);

private:
   void releaseHandles();
   bool ensurePool();
   mongoc_client_t *acquireClient();
   void releaseClient(mongoc_client_t *client);

   std::string getElementValue(const bson_iter_t &iter);
   DataTypes getElementType(const bson_iter_t &iter);
   bool validateConnectionParams();
   bool containsForbiddenOperator(const std::string &json, std::string &outOp);
   bool validateFind();
   bool validateAggregate();
   bson_t *buildFilter();
   bson_t *buildAggregatePipeline();
   void fillDestList(const bson_t *doc, EXTqlist *rowList);
   bool fetchCursorResults(mongoc_cursor_t *cursor, EXTqlist *retList);
};

#endif
