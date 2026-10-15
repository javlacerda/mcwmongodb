// MCWMongoDB - Omnis / MongoDB Integration
// DLL Entry point

#include "MCWMongoDB.h"
#include "Utils.h"
#include <map>
#include <vector>
#include <cstring>
#include <ctime>
#include <sstream>
#include <system_error>
#include <thread>

// List of objects provided by the xcomp
ECOobject oMongoDBObjects[] =
{
    {cObject_MongoDB, 2000, 0, 0}
};

// Params information
ECOparam paramsSetConnectionString[] =
{
    {7000, fftCharacter, 0, 0}
};

ECOparam paramsSetDatabase[] =
{
    {7001, fftCharacter, 0, 0}
};

ECOparam paramsSetCollection[] =
{
    {7002, fftCharacter, 0, 0}
};

ECOparam paramsFind[] =
{
    {7003, fftList, 0, 0}
};

ECOparam paramsSetFilter[] =
{
    {7008, fftCharacter, 0, 0}
};

ECOparam paramsSetOpts[] =
{
    {7010, fftCharacter, 0, 0}
};

ECOparam paramsSetAggregate[] =
{
    {7011, fftCharacter, 0, 0}
};

// List of functions
ECOmethodEvent oMongoDBFunctions[] =
{
    {cSetConnectionStringFunction,  6000, fftBoolean,   std::size(paramsSetConnectionString), paramsSetConnectionString, 0, 0},
    {cSetDatabaseFunction,          6001, fftBoolean,   std::size(paramsSetDatabase),         paramsSetDatabase,         0, 0},
    {cSetCollectionFunction,        6002, fftBoolean,   std::size(paramsSetCollection),       paramsSetCollection,       0, 0},
    {cGetErrorMessageFunction,      6003, fftCharacter, 0, 0, 0, 0},
    {cFindFunction,                 6004, fftBoolean,   std::size(paramsFind),                paramsFind,                0, 0},
    {cSetFilterFunction,            6006, fftBoolean,   std::size(paramsSetFilter),           paramsSetFilter,           0, 0},
    {cSetOptsFunction,              6009, fftBoolean,   std::size(paramsSetOpts),             paramsSetOpts,             0, 0 },
    {cSetAggregateFunction,         6010, fftBoolean,   std::size(paramsSetAggregate),        paramsSetAggregate,        0, 0 },
    {cAggregateFunction,            6011, fftBoolean,   std::size(paramsFind),                paramsFind,                0, 0 }
};

const qshort cFunctionsCount = static_cast<qshort>(std::size(oMongoDBFunctions));
const qshort cObjectsCount = static_cast<qshort>(std::size(oMongoDBObjects));

// mongoc_init / mongoc_cleanup are global-once
// mongoc_init() is idempotent and must be called once per process.
// We use a local static flag guarded by the DLL load sequence.
static void ensureMongoInit()
{
   static bool initialised = false;
   if (!initialised) {
      mongoc_init();
      initialised = true;
   }
}

// DLL entry point
extern "C" LRESULT OMNISWNDPROC MCWMongoDBWndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo *eci)
{
   ECOsetupCallbacks(hwnd, eci);

   switch (Msg) {
   case ECM_CONNECT:
      return EXT_FLAG_LOADED | EXT_FLAG_ALWAYS_USABLE | EXT_FLAG_REMAINLOADED | EXT_FLAG_NVOBJECTS;

   case ECM_GETOBJECT:
      return ECOreturnObjects(gInstLib, eci, &oMongoDBObjects[0], cObjectsCount);

   case ECM_OBJCONSTRUCT:
      if (eci->mCompId == cObject_MongoDB) {
         CMongoDB *mongoDb = reinterpret_cast<CMongoDB *>(ECOfindNVObject(eci->mOmnisInstance, lParam));
         if (mongoDb == nullptr) {
            CMongoDB *obj = new CMongoDB(reinterpret_cast<qobjinst>(lParam));
            ECOinsertNVObject(eci->mOmnisInstance, lParam, reinterpret_cast<void *>(obj));
         }
         return qtrue;
      }
      return qfalse;

   case ECM_OBJDESTRUCT:
      if (eci->mCompId == cObject_MongoDB && wParam == ECM_WPARAM_OBJINFO) {
         void *object = ECOremoveNVObject(eci->mOmnisInstance, lParam);
         if (object) {
            CMongoDB *obj = reinterpret_cast<CMongoDB *>(object);
            delete obj;
         }
      }
      return qtrue;

   case ECM_OBJECT_COPY: {
      objCopyInfo *copyInfo = reinterpret_cast<objCopyInfo *>(lParam);
      CMongoDB *srcObject = reinterpret_cast<CMongoDB *>(ECOfindNVObject(eci->mOmnisInstance, copyInfo->mSourceObject));
      if (srcObject) {
         CMongoDB *destObj = reinterpret_cast<CMongoDB *>(ECOfindNVObject(eci->mOmnisInstance, copyInfo->mDestinationObject));
         if (destObj == nullptr) {
            destObj = new CMongoDB(reinterpret_cast<qobjinst>(copyInfo->mDestinationObject), srcObject);
            ECOinsertNVObject(eci->mOmnisInstance, copyInfo->mDestinationObject, reinterpret_cast<void *>(destObj));
         }
         else {
            destObj->setObject(reinterpret_cast<qobjinst>(copyInfo->mDestinationObject), srcObject);
         }
         return qtrue;
      }
      return qfalse;
   }

   case ECM_GETMETHODNAME:
      if (eci->mCompId == cObject_MongoDB)
         return ECOreturnMethods(gInstLib, eci, &oMongoDBFunctions[0], cFunctionsCount);
      return qtrue;

   case ECM_METHODCALL: {
      void *object = ECOfindNVObject(eci->mOmnisInstance, lParam);
      if (object && eci->mCompId == cObject_MongoDB) {
         CMongoDB *mongoDb = reinterpret_cast<CMongoDB *>(object);
         return mongoDb->methodCall(eci);
      }
      return qfalse;
   }

   case ECM_CONSTPREFIX: {
      EXTfldval exfldval;
      ECOaddParam(eci, &exfldval);
      str80 conPrefix;
      RESloadString(gInstLib, RES_CONST_PREFIX, conPrefix);
      exfldval.setChar(conPrefix);
      return qtrue;
   }

   case ECM_GETCONSTNAME:
      return ECOreturnConstants(gInstLib, eci, RES_CONST_START, RES_CONST_END);

   case ECM_GETCOMPLIBINFO:
      return ECOreturnCompInfo(gInstLib, eci, LIB_RES_NAME, 0);

   case ECM_ISUNICODE:
      return qtrue;

   case ECM_GETVERSION:
      return ECOreturnVersion(VERSION_MAJOR, VERSION_MINOR);
   }

   return WNDdefWindowProc(hwnd, Msg, wParam, lParam, eci);
}

// Constructor
CMongoDB::CMongoDB(qobjinst pObjPtr)
   : m_ObjPtr(pObjPtr),
   m_Pool(nullptr)
{
   ensureMongoInit();
}

// Copy constructor
CMongoDB::CMongoDB(qobjinst pObjPtr, const CMongoDB *sourceObject)
   : m_ObjPtr(pObjPtr),
   m_Pool(nullptr)
{
   ensureMongoInit();
   setObject(pObjPtr, sourceObject);
}

// Copy the connection/query configuration (connection string, database, collection,
// filter, opts, aggregate pipeline).
// The live connection handles are NOT copied; each instance manages its own connection.
void CMongoDB::setObject(qobjinst pObjPtr, const CMongoDB *sourceObject)
{
   releaseHandles();
   m_ObjPtr = pObjPtr;
   m_connectionString = sourceObject->m_connectionString;
   m_databaseName = sourceObject->m_databaseName;
   m_collectionName = sourceObject->m_collectionName;
   m_filter = sourceObject->m_filter;
   m_opts = sourceObject->m_opts;
   m_aggregatePipeline = sourceObject->m_aggregatePipeline;
}

// Destructor - release mongoc handles
CMongoDB::~CMongoDB()
{
   releaseHandles();
}

// Release the mongoc client pool, if any.
// Must not be called while a client checked out via acquireClient() is still
// outstanding (find()/aggregate() always pop/push within the same call).
// mongoc_client_pool_destroy() joins the monitoring threads, and a monitor stuck
// connecting to an unreachable member (e.g. an arbiter advertised by the replica
// set but firewalled from the client) blocks it for up to connectTimeoutMS (10s).
// The pool is therefore destroyed on a detached thread so Omnis does not wait.
void CMongoDB::releaseHandles()
{
   if (m_Pool) {
      mongoc_client_pool_t *pool = m_Pool;
      m_Pool = nullptr;
      try {
         std::thread([pool]() { mongoc_client_pool_destroy(pool); }).detach();
      }
      catch (const std::system_error &) {
         mongoc_client_pool_destroy(pool);
      }
   }
}

// Method dispatch 
qbool CMongoDB::methodCall(EXTCompInfo *pEci)
{
   qlong funcId = ECOgetId(pEci);

   if (funcId == cGetErrorMessageFunction) {
      getErrorMessage(pEci);
      return qtrue;
   }

   EXTfldval rtnVal;
   qbool rtnCode = qfalse;
   m_errorMessage = "";

   switch (funcId) {
   case cSetConnectionStringFunction:
      rtnCode = setConnectionString(pEci);
      break;
   case cSetDatabaseFunction:
      rtnCode = setDatabase(pEci);
      break;
   case cSetCollectionFunction:
      rtnCode = setCollection(pEci);
      break;
   case cFindFunction:
      rtnCode = find(pEci);
      break;
   case cSetFilterFunction:
      rtnCode = setFilter(pEci);
      break;
   case cSetOptsFunction:
      rtnCode = setOpts(pEci);
      break;
   case cSetAggregateFunction:
      rtnCode = setAggregate(pEci);
      break;
   case cAggregateFunction:
      rtnCode = aggregate(pEci);
      break;
   }

   rtnVal.setBool((rtnCode == qtrue) ? preBoolTrue : preBoolFalse);
   ECOaddParam(pEci, &rtnVal);
   return rtnCode;
}

// Getter for the last error message.
void CMongoDB::getErrorMessage(EXTCompInfo *pEci) const
{
   EXTfldval result;
   stringToEXTfldval(m_errorMessage, result);
   ECOaddParam(pEci, &result);
}

// Setter for connection string to MongoDB.
qbool CMongoDB::setConnectionString(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_connectionString)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }

   // Drop any existing connection – it is now stale.
   releaseHandles();
   return qtrue;
}

// Setter for database name.
qbool CMongoDB::setDatabase(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_databaseName)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

// Setter for collection name.
qbool CMongoDB::setCollection(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_collectionName)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

// Setter for the filter (raw JSON) — the only way to filter find() results.
qbool CMongoDB::setFilter(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_filter)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

// Setter for find options (raw JSON, e.g. sort, limit, projection).
qbool CMongoDB::setOpts(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_opts)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

// Setter for the aggregation pipeline (raw JSON array of stages).
qbool CMongoDB::setAggregate(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_aggregatePipeline)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

// Execute a find query using the filter and return the results in the provided EXTqlist.
qbool CMongoDB::find(EXTCompInfo *pEci)
{
   if (!validateFind())
      return qfalse;

   std::unique_ptr<EXTqlist> retList(getListFromEXTCompInfo(pEci, 1, false));
   if (!retList) {
      m_errorMessage = "It was not possible to access the parameter number 1 or it is not a list.";
      return qfalse;
   }

   mongoc_client_t *client = acquireClient();
   if (!client)
      return qfalse;

   mongoc_collection_t *collection =
      mongoc_client_get_collection(client, m_databaseName.c_str(), m_collectionName.c_str());

   retList->clear(listVlen);
   addRowColToEXTqlist(retList.get(), "Documents");

   bson_t *filter = buildFilter();
   if (!filter) {
      mongoc_collection_destroy(collection);
      releaseClient(client);
      return qfalse;
   }

   bson_t *opts = nullptr;
   if (!m_opts.empty()) {
      bson_error_t optsErr{};
      opts = bson_new_from_json(reinterpret_cast<const uint8_t *>(m_opts.c_str()), static_cast<ssize_t>(m_opts.size()), &optsErr);
      if (!opts) {
         m_errorMessage = std::string("Invalid opts JSON: ") + optsErr.message;
         bson_destroy(filter);
         mongoc_collection_destroy(collection);
         releaseClient(client);
         return qfalse;
      }
   }

   mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, filter, opts, nullptr);
   bson_destroy(filter);
   if (opts) {
      bson_destroy(opts);
   }

   const bool ok = fetchCursorResults(cursor, retList.get());

   mongoc_collection_destroy(collection);
   releaseClient(client);

   if (!ok)
      return qfalse;

   ECOsetParameterChanged(pEci, 1);
   return qtrue;
}

// Execute the aggregation pipeline set via setAggregate() and return the results in the provided EXTqlist.
qbool CMongoDB::aggregate(EXTCompInfo *pEci)
{
   if (!validateAggregate())
      return qfalse;

   std::unique_ptr<EXTqlist> retList(getListFromEXTCompInfo(pEci, 1, false));
   if (!retList) {
      m_errorMessage = "It was not possible to access the parameter number 1 or it is not a list.";
      return qfalse;
   }

   mongoc_client_t *client = acquireClient();
   if (!client)
      return qfalse;

   mongoc_collection_t *collection =
      mongoc_client_get_collection(client, m_databaseName.c_str(), m_collectionName.c_str());

   retList->clear(listVlen);
   addRowColToEXTqlist(retList.get(), "Documents");

   bson_t *pipeline = buildAggregatePipeline();
   if (!pipeline) {
      mongoc_collection_destroy(collection);
      releaseClient(client);
      return qfalse;
   }

   mongoc_cursor_t *cursor = mongoc_collection_aggregate(collection, MONGOC_QUERY_NONE, pipeline, nullptr, nullptr);
   bson_destroy(pipeline);

   const bool ok = fetchCursorResults(cursor, retList.get());

   mongoc_collection_destroy(collection);
   releaseClient(client);

   if (!ok)
      return qfalse;

   ECOsetParameterChanged(pEci, 1);
   return qtrue;
}

// Consume a cursor into retList, using the same row-building logic as find()/aggregate().
// Destroys the cursor before returning either way.
bool CMongoDB::fetchCursorResults(mongoc_cursor_t *cursor, EXTqlist *retList)
{
   const bson_t *doc = nullptr;
   bson_error_t  err{};

   while (mongoc_cursor_next(cursor, &doc)) {
      std::unique_ptr<EXTqlist> rowList(new EXTqlist(listVlen));
      rowList->setRow(qtrue);

      fillDestList(doc, rowList.get());

      qlong numLin = retList->insertRow();
      listToEXTqlist(numLin, 1, rowList.get(), retList, true);
   }

   if (mongoc_cursor_error(cursor, &err)) {
      m_errorMessage = err.message;
      mongoc_cursor_destroy(cursor);
      return false;
   }

   mongoc_cursor_destroy(cursor);
   return true;
}

//
// Internal helper methods for building filters and executing queries
//

// Lazily create the mongoc client pool for the current connection string.
// Using a pool (rather than a plain mongoc_client_t) means replica-set
// server discovery happens in background monitoring threads that watch all
// seed hosts concurrently, so a single unreachable/slow seed no longer
// blocks every operation for its full connectTimeoutMS.
// Returns false and sets m_errorMessage on failure.
bool CMongoDB::ensurePool()
{
   if (m_Pool)
      return true;

   bson_error_t err;
   mongoc_uri_t *uri = mongoc_uri_new_with_error(m_connectionString.c_str(), &err);
   if (!uri) {
      m_errorMessage = std::string("Invalid URI: ") + err.message;
      return false;
   }

   m_Pool = mongoc_client_pool_new_with_error(uri, &err);
   mongoc_uri_destroy(uri);

   if (!m_Pool) {
      m_errorMessage = std::string("Failed to create mongoc client pool: ") + err.message;
      return false;
   }

   mongoc_client_pool_set_appname(m_Pool, "MCWMongoDB");
   return true;
}

// Check out a client from the pool (creating the pool if needed).
// Caller must pass the returned client to releaseClient() when done.
// Returns nullptr and sets m_errorMessage on failure.
mongoc_client_t *CMongoDB::acquireClient()
{
   if (!ensurePool())
      return nullptr;

   mongoc_client_t *client = mongoc_client_pool_pop(m_Pool);
   if (!client) {
      m_errorMessage = "Failed to acquire a Mongo client from the pool.";
   }
   return client;
}

// Return a client acquired via acquireClient() to the pool.
void CMongoDB::releaseClient(mongoc_client_t *client)
{
   if (client && m_Pool) {
      mongoc_client_pool_push(m_Pool, client);
   }
}

// Validate that the connection string, database and collection are all set.
bool CMongoDB::validateConnectionParams()
{
   if (m_connectionString.empty())
   {
      m_errorMessage = "Connection string not defined.";
      return false;
   }
   if (m_databaseName.empty()) {
      m_errorMessage = "Database not defined.";
      return false;
   }
   if (m_collectionName.empty()) {
      m_errorMessage = "Collection not defined.";
      return false;
   }
   return true;
}

// Check raw user-supplied JSON (filter / aggregate pipeline) for operators
// that allow arbitrary code execution or writes ($where/$function/$accumulator/$merge/$out).
bool CMongoDB::containsForbiddenOperator(const std::string &json, std::string &outOp)
{
   static const std::vector<std::string> FORBIDDEN_OPERATORS = {
       "$where", "$function", "$accumulator", "$merge", "$out"
   };

   for (const std::string &op : FORBIDDEN_OPERATORS) {
      if (json.find(op) != std::string::npos) {
         outOp = op;
         return true;
      }
   }
   return false;
}

// Validate the current filter configuration before running a find.
bool CMongoDB::validateFind()
{
   if (!validateConnectionParams())
      return false;

   std::string forbiddenOp;
   if (containsForbiddenOperator(m_filter, forbiddenOp)) {
      m_errorMessage = "Filter contains invalid operator (" + forbiddenOp + ")";
      return false;
   }
   return true;
}

// Validate the current aggregation pipeline configuration before running an aggregate.
bool CMongoDB::validateAggregate()
{
   if (!validateConnectionParams())
      return false;

   if (m_aggregatePipeline.empty()) {
      m_errorMessage = "Aggregate pipeline not defined.";
      return false;
   }

   std::string forbiddenOp;
   if (containsForbiddenOperator(m_aggregatePipeline, forbiddenOp)) {
      m_errorMessage = "Aggregate pipeline contains invalid operator (" + forbiddenOp + ")";
      return false;
   }
   return true;
}

// Build the aggregation pipeline document, e.g. {"pipeline": [ {"$match": {...}}, ... ]}.
// Returns a heap-allocated bson_t; caller must bson_destroy() it.
bson_t *CMongoDB::buildAggregatePipeline()
{
   std::string json = "{\"pipeline\":" + m_aggregatePipeline + "}";
   bson_error_t err{};
   bson_t *doc = bson_new_from_json(reinterpret_cast<const uint8_t *>(json.c_str()), static_cast<ssize_t>(json.size()), &err);
   if (!doc) {
      m_errorMessage = std::string("Invalid aggregate pipeline JSON: ") + err.message;
      return nullptr;
   }
   return doc;
}

// Build the filter document from the filter (raw JSON), if present.
// Returns a heap-allocated bson_t; caller must bson_destroy() it.
bson_t *CMongoDB::buildFilter()
{
   if (m_filter.empty()) {
      return bson_new();
   }

   bson_error_t err{};
   bson_t *doc = bson_new_from_json(reinterpret_cast<const uint8_t *>(m_filter.c_str()), static_cast<ssize_t>(m_filter.size()), &err);
   if (!doc) {
      m_errorMessage = std::string("Invalid filter JSON: ") + err.message;
      return nullptr;
   }
   return doc;
}

// Fill the provided EXTqlist with the fields from the given BSON document.
void CMongoDB::fillDestList(const bson_t *doc, EXTqlist *rowList)
{
   std::map<std::string, qshort> colPos;
   std::map<std::string, DataTypes> colTypes;

   bson_iter_t iter;
   if (bson_iter_init(&iter, doc)) {
      while (bson_iter_next(&iter)) {
         std::string fieldName = bson_iter_key(&iter);
         DataTypes type = getElementType(iter);

         switch (type) {
         case dtINTEGER_32:
            colPos[fieldName] = addIntegerColToEXTqlist(rowList, fieldName.c_str());
            colTypes[fieldName] = dtINTEGER_32;
            break;
         case dtINTEGER_64:
            colPos[fieldName] = addInteger64ColToEXTqlist(rowList, fieldName.c_str());
            colTypes[fieldName] = dtINTEGER_64;
            break;
         case dtBOOLEAN:
            colPos[fieldName] = addBoolColToEXTqlist(rowList, fieldName.c_str());
            colTypes[fieldName] = dtBOOLEAN;
            break;
         case dtDOUBLE:
            colPos[fieldName] = addDoubleColToEXTqlist(rowList, fieldName.c_str());
            colTypes[fieldName] = dtDOUBLE;
            break;
         case dtDATE_TIME:
            colPos[fieldName] = addDateTimeColToEXTqlist(rowList, fieldName.c_str());
            colTypes[fieldName] = dtDATE_TIME;
            break;
         default:
            colPos[fieldName] = addCharacterColToEXTqlist(rowList, maxcharlen, fieldName.c_str());
            colTypes[fieldName] = dtCHARACTER;
            break;
         }
      }
   }

   qlong newRow = rowList->insertRow();

   if (bson_iter_init(&iter, doc)) {
      while (bson_iter_next(&iter)) {
         std::string fieldName = bson_iter_key(&iter);
         DataTypes type = colTypes.count(fieldName) ? colTypes[fieldName] : dtCHARACTER;

         switch (type) {
         case dtINTEGER_32:
            intToEXTqlist(newRow, colPos[fieldName], static_cast<int>(bson_iter_int32(&iter)), rowList);
            break;
         case dtINTEGER_64:
            longToEXTqlist(newRow, colPos[fieldName], bson_iter_int64(&iter), rowList);
            break;
         case dtBOOLEAN:
            longToEXTqlist(newRow, colPos[fieldName], static_cast<long>(bson_iter_bool(&iter)), rowList);
            break;
         case dtDOUBLE:
            doubleToEXTqlist(newRow, colPos[fieldName], bson_iter_double(&iter), rowList);
            break;
         case dtDATE_TIME:
            dateTimeToEXTqlist(newRow, colPos[fieldName], std::chrono::milliseconds(bson_iter_date_time(&iter)), rowList);
            break;
         case dtNULL:
            nullToEXTqlist(newRow, colPos[fieldName], rowList);
            break;
         default:
            stringToEXTqlist(newRow, colPos[fieldName], getElementValue(iter), rowList);
            break;
         }
      }
   }
}

// Convert a BSON element to a string representation for display in the list.
std::string CMongoDB::getElementValue(const bson_iter_t &iter)
{
   switch (bson_iter_type(&iter)) {
   case BSON_TYPE_INT32:
      return std::to_string(bson_iter_int32(&iter));

   case BSON_TYPE_INT64:
      return std::to_string(bson_iter_int64(&iter));

   case BSON_TYPE_DOUBLE:
      return std::to_string(bson_iter_double(&iter));

   case BSON_TYPE_BOOL:
      return bson_iter_bool(&iter) ? "true" : "false";

   case BSON_TYPE_UTF8: {
      uint32_t len = 0;
      const char *s = bson_iter_utf8(&iter, &len);
      return std::string(s, len);
   }

   case BSON_TYPE_DATE_TIME: {
      int64_t ms = bson_iter_date_time(&iter);
      std::time_t t = static_cast<std::time_t>(ms / 1000);
      std::tm tm{};
#if defined(_WIN32)
      gmtime_s(&tm, &t);
#else
      gmtime_r(&t, &tm);
#endif
      char buffer[64];
      std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
      return buffer;
   }

   case BSON_TYPE_NULL:
      return "null";

   case BSON_TYPE_DOCUMENT: {
      const uint8_t *data = nullptr;
      uint32_t len = 0;
      bson_iter_document(&iter, &len, &data);

      bson_t  sub = BSON_INITIALIZER;
      bson_init_static(&sub, data, len);

      char *json = bson_as_relaxed_extended_json(&sub, nullptr);
      std::string result(json);
      bson_free(json);
      return result;
   }

   case BSON_TYPE_ARRAY: {
      //const uint8_t *data = nullptr;
      //uint32_t len = 0;
      //bson_iter_array(&iter, &len, &data);
      //bson_t arr = BSON_INITIALIZER;
      //bson_init_static(&arr, data, len);
      //char *json = bson_as_relaxed_extended_json(&arr, nullptr);
      //std::string result(json);
      //bson_free(json);
      //return result;

      const uint8_t *data = nullptr;
      uint32_t len = 0;

      bson_iter_array(&iter, &len, &data);

      bson_t arr;
      bson_init_static(&arr, data, len);

      bson_iter_t child;
      bson_iter_init(&child, &arr);

      std::ostringstream oss;
      oss << "[";

      bool first = true;

      while (bson_iter_next(&child)) {
         if (!first)
            oss << ",";

         first = false;

         if (BSON_ITER_HOLDS_DOCUMENT(&child)) {
            const uint8_t *subdata = nullptr;
            uint32_t sublen = 0;
            bson_iter_document(&child, &sublen, &subdata);

            bson_t subdoc;
            bson_init_static(&subdoc, subdata, sublen);

            char *json = bson_as_relaxed_extended_json(&subdoc, nullptr);
            oss << json;
            bson_free(json);
         }
      }

      oss << "]";
      return oss.str();
   }

   case BSON_TYPE_OID: {
      char oidStr[25];
      bson_oid_to_string(bson_iter_oid(&iter), oidStr);
      return oidStr;
   }

   case BSON_TYPE_DECIMAL128: {
      bson_decimal128_t dec;
      bson_iter_decimal128(&iter, &dec);
      char buf[BSON_DECIMAL128_STRING];
      bson_decimal128_to_string(&dec, buf);
      return buf;
   }

   default:
      return "<unsupported bson type>";
   }
}

// Determine the DataType to use in the EXTqlist based on the BSON type.
DataTypes CMongoDB::getElementType(const bson_iter_t &iter)
{
   switch (bson_iter_type(&iter)) {
   case BSON_TYPE_INT32:
      return dtINTEGER_32;
   case BSON_TYPE_INT64:
      return dtINTEGER_64;
   case BSON_TYPE_DECIMAL128:
   case BSON_TYPE_DOUBLE:
      return dtDOUBLE;
   case BSON_TYPE_BOOL:
      return dtBOOLEAN;
   case BSON_TYPE_DATE_TIME:
      return dtDATE_TIME;
   case BSON_TYPE_NULL:
      return dtNULL;
   default:
      return dtCHARACTER;
   }
}
