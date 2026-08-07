// MCWMongoDB - Omnis / MongoDB Integration
// DLL Entry point

#include "MCWMongoDB.h"
#include "../Utils.h"
#include <map>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <sstream>

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

ECOparam paramsAddFilter[] =
{
    {7004, fftCharacter, 0, 0},
    {7005, fftInteger, 0, 0},
    {7006, fftInteger, 0, 0},
    {7007, fftInteger, 0, 0}
};

ECOparam paramsSetDynamicFilter[] =
{
    {7008, fftCharacter, 0, 0}
};

ECOparam paramsAddInFilter[] =
{
    {7008, fftCharacter, 0, 0},
    {7005, fftInteger,   0, 0},
    {7009, fftInteger,   0, 0},
    {7007, fftList,      0, 0}
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
    {cAddFilterFunction,            6005, fftBoolean,   std::size(paramsAddFilter),           paramsAddFilter,           0, 0},
    {cSetDynamicFilterFunction,     6006, fftBoolean,   std::size(paramsSetDynamicFilter),    paramsSetDynamicFilter,    0, 0},
    {cClearFiltersFunction,         6007, fftBoolean,   0, 0, 0, 0},
    {cAddInFilterFunction,          6008, fftBoolean,   std::size(paramsAddInFilter),         paramsAddInFilter,         0, 0 },
    {cSetOptsFunction,              6009, fftBoolean,   std::size(paramsSetOpts),             paramsSetOpts,             0, 0 },
    {cSetAggregateFunction,         6010, fftBoolean,   std::size(paramsSetAggregate),        paramsSetAggregate,        0, 0 },
    {cAggregateFunction,            6011, fftBoolean,   std::size(paramsFind),                paramsFind,                0, 0 },
    {cClearAggregateFunction,       6012, fftBoolean,   0, 0, 0, 0 }
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
   m_Client(nullptr),
   m_Database(nullptr),
   m_Collection(nullptr)
{
   ensureMongoInit();
}

// Copy constructor
CMongoDB::CMongoDB(qobjinst pObjPtr, const CMongoDB *sourceObject)
   : m_ObjPtr(pObjPtr),
   m_Client(nullptr),
   m_Database(nullptr),
   m_Collection(nullptr)
{
   ensureMongoInit();
   setObject(pObjPtr, sourceObject);
}

// Copy the connection/query configuration (connection string, database, collection,
// filters, dynamic filter, opts, aggregate pipeline).
// The live connection handles are NOT copied; each instance manages its own connection.
void CMongoDB::setObject(qobjinst pObjPtr, const CMongoDB *sourceObject)
{
   releaseHandles();
   m_ObjPtr = pObjPtr;
   m_connectionString = sourceObject->m_connectionString;
   m_databaseName = sourceObject->m_databaseName;
   m_collectionName = sourceObject->m_collectionName;
   m_filters = sourceObject->m_filters;
   m_dynamicFilter = sourceObject->m_dynamicFilter;
   m_opts = sourceObject->m_opts;
   m_aggregatePipeline = sourceObject->m_aggregatePipeline;
}

// Destructor - release mongoc handles
CMongoDB::~CMongoDB()
{
   releaseHandles();
}

// Release all live mongoc handles in the correct order.
void CMongoDB::releaseHandles()
{
   if (m_Collection) {
      mongoc_collection_destroy(m_Collection);
      m_Collection = nullptr;
   }
   if (m_Database) {
      mongoc_database_destroy(m_Database);
      m_Database = nullptr;
   }
   if (m_Client) {
      mongoc_client_destroy(m_Client);
      m_Client = nullptr;
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
   case cAddFilterFunction:
      rtnCode = addFilter(pEci);
      break;
   case cAddInFilterFunction:
      rtnCode = addInFilter(pEci);
      break;
   case cSetDynamicFilterFunction:
      rtnCode = setDynamicFilter(pEci);
      break;
   case cClearFiltersFunction:
      rtnCode = clearFilters(pEci);
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
   case cClearAggregateFunction:
      rtnCode = clearAggregate(pEci);
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

// Setter for dynamic filter (raw JSON).
// This is added as-is to the filter document, so the user can use it for complex queries that are not possible with the simple addFilter method.
qbool CMongoDB::setDynamicFilter(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_dynamicFilter)) {
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

// Add a filter item to the list of filters to apply on find.
qbool CMongoDB::addFilter(EXTCompInfo *pEci)
{
   std::string fieldName;
   if (!getStringFromEXTCompInfo(pEci, 1, fieldName)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }

   int oper = 0;
   if (!getIntFromEXTCompInfo(pEci, 2, oper)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   if (oper < 0 || oper >= static_cast<int>(foCOUNT)) {
      m_errorMessage = "Invalid operator.";
      return qfalse;
   }

   std::string fieldValue;
   if (!getStringFromEXTCompInfo(pEci, 3, fieldValue)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }

   int type = 0;
   if (!getIntFromEXTCompInfo(pEci, 4, type)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   if (type < 0 || type >= static_cast<int>(dtCOUNT)) {
      m_errorMessage = "Invalid data type.";
      return qfalse;
   }

   m_filters.push_back(FilterItem{ fieldName, fieldValue, static_cast<FilterOperators>(oper), static_cast<DataTypes>(type) });
   return qtrue;
}

// Clear all filters from the list and also the dynamic filter.
qbool CMongoDB::clearFilters(EXTCompInfo *)
{
   m_filters.clear();
   m_dynamicFilter.clear();
   return qtrue;
}

// Clear the aggregation pipeline set via setAggregate().
qbool CMongoDB::clearAggregate(EXTCompInfo *)
{
   m_aggregatePipeline.clear();
   return qtrue;
}

// Add an IN / NOT_IN filter.
qbool CMongoDB::addInFilter(EXTCompInfo *pEci)
{
   std::string fieldName;
   if (!getStringFromEXTCompInfo(pEci, 1, fieldName)) {
      m_errorMessage = "Missing fieldName parameter.";
      return qfalse;
   }

   int oper = 0;
   if (!getIntFromEXTCompInfo(pEci, 2, oper)) {
      m_errorMessage = "Missing operator parameter.";
      return qfalse;
   }
   if (oper != foIN && oper != foNOT_IN) {
      m_errorMessage = "Operator must be IN or NOT_IN.";
      return qfalse;
   }

   std::unique_ptr<EXTqlist> omnisList(getListFromEXTCompInfo(pEci, 3, false));
   if (!omnisList) {
      m_errorMessage = "It was not possible to access the parameter number 4 or it is not a list.";
      return qfalse;
   }
   if (omnisList->colCnt() == 0) {
      m_errorMessage = "The values list must have, at least, one column.";
      return qfalse;
   }

   std::vector<std::string> values;
   for (qlong row = 1; row <= omnisList->rowCnt(); ++row) {
      std::string value;
      getStringFromEXTqlist(row, 1, omnisList.get(), value);
      values.push_back(std::move(value));
   }

   int type = 0;
   if (!getIntFromEXTCompInfo(pEci, 4, type)) {
      m_errorMessage = "Missing data type parameter.";
      return qfalse;
   }
   if (type < 0 || type >= static_cast<int>(dtCOUNT)) {
      m_errorMessage = "Invalid data type.";
      return qfalse;
   }

   m_filters.push_back(FilterItem{ fieldName, values, static_cast<FilterOperators>(oper), static_cast<DataTypes>(type) });
   return qtrue;
}

// Execute a find query with the current filters and return the results in the provided EXTqlist.
qbool CMongoDB::find(EXTCompInfo *pEci)
{
   if (!validateFind())
      return qfalse;

   std::unique_ptr<EXTqlist> retList(getListFromEXTCompInfo(pEci, 1, false));
   if (!retList) {
      m_errorMessage = "It was not possible to access the parameter number 1 or it is not a list.";
      return qfalse;
   }

   if (!openCollection())
      return qfalse;

   retList->clear(listVlen);
   addRowColToEXTqlist(retList.get(), "Documents");

   bson_t *filter = buildFilter();
   if (!filter) {
      return qfalse;
   }

   bson_t *opts = nullptr;
   if (!m_opts.empty()) {
      bson_error_t optsErr{};
      opts = bson_new_from_json(reinterpret_cast<const uint8_t *>(m_opts.c_str()), static_cast<ssize_t>(m_opts.size()), &optsErr);
      if (!opts) {
         m_errorMessage = std::string("Invalid opts JSON: ") + optsErr.message;
         bson_destroy(filter);
         return qfalse;
      }
   }

   mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(m_Collection, filter, opts, nullptr);
   bson_destroy(filter);
   if (opts) {
      bson_destroy(opts);
   }

   if (!fetchCursorResults(cursor, retList.get()))
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

   if (!openCollection())
      return qfalse;

   retList->clear(listVlen);
   addRowColToEXTqlist(retList.get(), "Documents");

   bson_t *pipeline = buildAggregatePipeline();
   if (!pipeline) {
      return qfalse;
   }

   mongoc_cursor_t *cursor = mongoc_collection_aggregate(m_Collection, MONGOC_QUERY_NONE, pipeline, nullptr, nullptr);
   bson_destroy(pipeline);

   if (!fetchCursorResults(cursor, retList.get()))
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

// Open (or reuse) the mongoc_client -> database -> collection chain.
// Returns false and sets errorMessage on failure.
bool CMongoDB::openCollection()
{
   // Reuse existing client if already open.
   if (!m_Client) {
      bson_error_t err;
      mongoc_uri_t *uri = mongoc_uri_new_with_error(m_connectionString.c_str(), &err);
      if (!uri) {
         m_errorMessage = std::string("Invalid URI: ") + err.message;
         return false;
      }

      m_Client = mongoc_client_new_from_uri(uri);
      mongoc_uri_destroy(uri);
      if (!m_Client) {
         m_errorMessage = "Failed to create mongoc client.";
         return false;
      }
      // Optional: set a server-selection timeout (5 s) so errors are fast.
      mongoc_client_set_appname(m_Client, "MCWMongoDB");
   }

   // Re-create database / collection handles if the names changed.
   if (!m_Database || mongoc_database_get_name(m_Database) != m_databaseName) {
      if (m_Collection) {
         mongoc_collection_destroy(m_Collection);
         m_Collection = nullptr;
      }
      if (m_Database) {
         mongoc_database_destroy(m_Database);
         m_Database = nullptr;
      }
      m_Database = mongoc_client_get_database(m_Client, m_databaseName.c_str());
   }

   // Re-create the collection handle if the collection name changed.
   if (m_Collection && mongoc_collection_get_name(m_Collection) != m_collectionName) {
      mongoc_collection_destroy(m_Collection);
      m_Collection = nullptr;
   }

   if (!m_Collection) {
      m_Collection = mongoc_database_get_collection(m_Database, m_collectionName.c_str());
   }

   return true;
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

// Check raw user-supplied JSON (dynamic filter / aggregate pipeline) for operators
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
   if (containsForbiddenOperator(m_dynamicFilter, forbiddenOp)) {
      m_errorMessage = "Dynamic filter contains invalid operator (" + forbiddenOp + ")";
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

// Build the complete filter document.
// Returns a heap-allocated bson_t; caller must bson_destroy() it.
bson_t *CMongoDB::buildFilter()
{
   std::vector<bson_t *> items;

   for (const FilterItem &itm : m_filters) {
      bson_t *item = bson_new();
      bool success = false;
      if (itm.oper == foIN || itm.oper == foNOT_IN) {
         success = buildInFilterItem(item, itm.fieldName, itm.oper, itm.values, itm.type);
      }
      else {
         success = buildFilterItem(item, itm.fieldName, itm.oper, itm.value, itm.type);
      }

      if (!success) {
         for (bson_t *b : items)
            bson_destroy(b);

         return nullptr;
      }

      items.push_back(item);
   }

   // Add the dynamic filter (raw JSON) if present.
   bson_t *dynDoc = nullptr;
   if (!m_dynamicFilter.empty()) {
      bson_error_t err{};
      dynDoc = bson_new_from_json(reinterpret_cast<const uint8_t *>(m_dynamicFilter.c_str()), static_cast<ssize_t>(m_dynamicFilter.size()), &err);
      if (dynDoc) {
         items.push_back(dynDoc);
      }
      else {
         m_errorMessage = std::string("Invalid dynamic filter JSON: ") + err.message;
         for (bson_t *b : items)
            bson_destroy(b);
         return nullptr;
      }
   }

   // No filters -> return empty document.
   if (items.empty()) {
      return bson_new();
   }

   // Single filter -> return it directly (no $and wrapper needed).
   if (items.size() == 1) {
      bson_t *result = bson_copy(items[0]);
      for (bson_t *b : items)
         bson_destroy(b);

      return result;
   }

   // Multiple filters -> wrap in { $and: [ ... ] }.
   bson_t *finalFilter = bson_new();
   bson_t andArray = BSON_INITIALIZER;
   BSON_APPEND_ARRAY_BEGIN(finalFilter, "$and", &andArray);

   char keyBuf[16];
   uint32_t idx = 0;
   for (bson_t *item : items) {
      const char *key;
      bson_uint32_to_string(idx++, &key, keyBuf, sizeof(keyBuf));
      BSON_APPEND_DOCUMENT(&andArray, key, item);
   }

   bson_append_array_end(finalFilter, &andArray);

   for (bson_t *b : items)
      bson_destroy(b);

   return finalFilter;
}

// Append a single field comparison to dest.
// dest must already be initialised by the caller (bson_new() or similar).
bool CMongoDB::buildFilterItem(bson_t *dest, const std::string &fieldName, FilterOperators filterOperator, const std::string &value, DataTypes type)
{
   const char *field = fieldName.c_str();

   if (filterOperator == foEQUAL) {
      // { field: <value> }
      return appendBsonValue(dest, fieldName, value, type);
   }

   // All other operators: { field: { $op: <value> } }
   const char *opStr = nullptr;
   switch (filterOperator) {
   case foNOT_EQUAL:
      opStr = "$ne";
      break;
   case foGREATER:
      opStr = "$gt";
      break;
   case foGREATER_OR_EQUAL:
      opStr = "$gte";
      break;
   case foLESSER:
      opStr = "$lt";
      break;
   case foLESSER_OR_EQUAL:
      opStr = "$lte";
      break;
   default:
      m_errorMessage = std::string("Unsupported operator: ") + std::to_string(filterOperator);
      return false;
   }

   bson_t opDoc = BSON_INITIALIZER;
   BSON_APPEND_DOCUMENT_BEGIN(dest, field, &opDoc);
   if (!appendBsonValue(&opDoc, opStr, value, type)) {
      return false;
   }

   bson_append_document_end(dest, &opDoc);
   return true;
}

// Build a { field: { $in: [v1, v2, ...] } } or $nin document.
// Dest must be an already-initialised bson_t (bson_new()).
bool CMongoDB::buildInFilterItem(bson_t *dest, const std::string &fieldName, FilterOperators filterOperator, const std::vector<std::string> &values, DataTypes type)
{
   const char *opStr = (filterOperator == foIN) ? "$in" : "$nin";

   // Build the values array first.
   bson_t valArray = BSON_INITIALIZER;
   bson_t opDoc = BSON_INITIALIZER;

   BSON_APPEND_DOCUMENT_BEGIN(dest, fieldName.c_str(), &opDoc);
   BSON_APPEND_ARRAY_BEGIN(&opDoc, opStr, &valArray);

   char keyBuf[16];
   uint32_t idx = 0;
   for (const std::string &val : values) {
      const char *key;
      bson_uint32_to_string(idx++, &key, keyBuf, sizeof(keyBuf));
      if (!appendBsonValue(&valArray, key, val, type))
         return false;
   }

   bson_append_array_end(&opDoc, &valArray);
   bson_append_document_end(dest, &opDoc);
   return true;
}

// Append a typed value to a BSON document under the given key.
bool CMongoDB::appendBsonValue(bson_t *doc, const std::string &key, const std::string &value, DataTypes type)
{
   const char *k = key.c_str();

   switch (type) {
   case dtCHARACTER:
      BSON_APPEND_UTF8(doc, k, value.c_str());
      break;
   case dtINTEGER_32:
      try {
         BSON_APPEND_INT32(doc, k, static_cast<int32_t>(std::stoi(value)));
      }
      catch (const std::exception &) {
         m_errorMessage = "Invalid 32-bit integer value for field '" + key + "': " + value;
         return false;
      }
      break;
   case dtINTEGER_64:
      try {
         BSON_APPEND_INT64(doc, k, static_cast<int64_t>(std::stoll(value)));
      }
      catch (const std::exception &) {
         m_errorMessage = "Invalid 64-bit integer value for field '" + key + "': " + value;
         return false;
      }
      break;
   case dtDOUBLE:
      try {
         BSON_APPEND_DOUBLE(doc, k, std::stod(value));
      }
      catch (const std::exception &) {
         m_errorMessage = "Invalid floating point value for field '" + key + "': " + value;
         return false;
      }
      break;
   case dtBOOLEAN:
      BSON_APPEND_BOOL(doc, k, value == "true" || value == "1");
      break;
   case dtNULL:
      BSON_APPEND_NULL(doc, k);
      break;
   case dtDATE_TIME: {
      int64_t ms;
      if (!iso8601ToMilliseconds(value, ms)) {
         return false;
      }

      BSON_APPEND_DATE_TIME(doc, k, ms);
      break;
   }

   default:
      m_errorMessage = std::string("Unsupported DataType: ") + std::to_string(type);
      return false;
   }

   return true;
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

// Parse an ISO8601 date string and convert it to milliseconds since the Unix epoch.
bool CMongoDB::iso8601ToMilliseconds(const std::string &dateTimeIso8601, int64_t &retMs)
{
   std::tm tm{};
   int milliseconds = 0;
   char tzSign = 'Z';
   int tzHour = 0;
   int tzMin = 0;

   // Try with milliseconds and timezone offset.
   int parsed = sscanf_s(
      dateTimeIso8601.c_str(),
      "%4d-%2d-%2dT%2d:%2d:%2d.%3d%c%2d:%2d",
      &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &milliseconds, &tzSign, 1, &tzHour, &tzMin);

   if (parsed < 7) {
      // Try without milliseconds.
      parsed = sscanf_s(
         dateTimeIso8601.c_str(),
         "%4d-%2d-%2dT%2d:%2d:%2d%c%2d:%2d",
         &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &tzSign, 1, &tzHour, &tzMin);
   }

   if (parsed < 6) {
      m_errorMessage = "Invalid ISO8601 date: " + dateTimeIso8601;
      return false;
   }

   tm.tm_year -= 1900;
   tm.tm_mon -= 1;
   tm.tm_isdst = 0;

   std::time_t utcTime = _mkgmtime(&tm);
   if (utcTime == -1) {
      m_errorMessage = "Failed to convert date: " + dateTimeIso8601;
      return false;
   }

   // Apply timezone offset.
   int64_t offsetSeconds = 0;
   if (tzSign == '+' || tzSign == '-') {
      offsetSeconds = tzHour * 3600LL + tzMin * 60LL;
      if (tzSign == '-') offsetSeconds = -offsetSeconds;
   }

   utcTime -= static_cast<std::time_t>(offsetSeconds);
   retMs = static_cast<int64_t>(utcTime) * 1000LL + milliseconds;
   return true;
}
