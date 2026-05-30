/*
** MCWMongoDB - Omnis / MongoDB Integration
** DLL Entry point
*/

#include "MCWMongoDB.h"
#include "Utils.h"
#include <map>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <sstream>

ECOobject oMongoDBObjects[] =
{
    {cObject_MongoDB, 2000, 0, 0}
};

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

ECOmethodEvent oMongoDBFunctions[] =
{
    {cSetConnectionStringFunction,  6000, fftBoolean,   sizeof(paramsSetConnectionString) / sizeof(ECOparam), paramsSetConnectionString, 0, 0},
    {cSetDatabaseFunction,          6001, fftBoolean,   sizeof(paramsSetDatabase) / sizeof(ECOparam), paramsSetDatabase,         0, 0},
    {cSetCollectionFunction,        6002, fftBoolean,   sizeof(paramsSetCollection) / sizeof(ECOparam), paramsSetCollection,       0, 0},
    {cGetErrorMessageFunction,      6003, fftCharacter, 0, 0, 0, 0},
    {cFindFunction,                 6004, fftBoolean,   sizeof(paramsFind) / sizeof(ECOparam), paramsFind,                0, 0},
    {cAddFilterFunction,            6005, fftBoolean,   sizeof(paramsAddFilter) / sizeof(ECOparam), paramsAddFilter,           0, 0},
    {cSetDynamicFilterFunction,     6006, fftBoolean,   sizeof(paramsSetDynamicFilter) / sizeof(ECOparam), paramsSetDynamicFilter,    0, 0},
    {cClearFiltersFunction,         6007, fftBoolean,   0, 0, 0, 0}
};

#define cFunctionsCount (sizeof(oMongoDBFunctions) / sizeof(ECOmethodEvent))
#define cObjectsCount   (sizeof(oMongoDBObjects)   / sizeof(ECOobject))

/*
** mongoc_init / mongoc_cleanup are global-once
** mongoc_init() is idempotent and must be called once per process.
** We use a local static flag guarded by the DLL load sequence.
*/
static void ensureMongoInit()
{
   static bool initialised = false;
   if (!initialised) {
      mongoc_init();
      initialised = true;
   }
}

/*
**  DLL entry point  (unchanged from original)
*/
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

/*
** Constructor / Destructor / Copy
*/
CMongoDB::CMongoDB(qobjinst pObjPtr)
   : m_ObjPtr(pObjPtr),
   m_Client(nullptr),
   m_Database(nullptr),
   m_Collection(nullptr)
{
   ensureMongoInit();
}

CMongoDB::CMongoDB(qobjinst pObjPtr, const CMongoDB *sourceObject)
   : m_ObjPtr(pObjPtr),
   m_Client(nullptr),
   m_Database(nullptr),
   m_Collection(nullptr)
{
   ensureMongoInit();
   setObject(pObjPtr, sourceObject);
}

/*
** Copy only the configuration strings
** the live connection handles are NOT copied; each instance manages its own connection.
*/
void CMongoDB::setObject(qobjinst pObjPtr, const CMongoDB *sourceObject)
{
   releaseHandles();
   m_ObjPtr = pObjPtr;
   m_connectionString = sourceObject->m_connectionString;
   m_database = sourceObject->m_database;
   m_collection = sourceObject->m_collection;
}

CMongoDB::~CMongoDB()
{
   releaseHandles();
}

/*
** Release all live mongoc handles in the correct order.
*/
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

/*
** Method dispatch
*/
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
   case cSetDynamicFilterFunction:
      rtnCode = setDynamicFilter(pEci);
      break;
   case cClearFiltersFunction:
      rtnCode = clearFilters(pEci);
      break;
   }

   rtnVal.setBool((rtnCode == qtrue) ? preBoolTrue : preBoolFalse);
   ECOaddParam(pEci, &rtnVal);
   return rtnCode;
}

/*
**  Public setters
*/
void CMongoDB::getErrorMessage(EXTCompInfo *pEci) const
{
   EXTfldval result;
   stringToEXTfldval(m_errorMessage, result);
   ECOaddParam(pEci, &result);
}

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

qbool CMongoDB::setDatabase(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_database)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

qbool CMongoDB::setCollection(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_collection)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

qbool CMongoDB::setDynamicFilter(EXTCompInfo *pEci)
{
   if (!getStringFromEXTCompInfo(pEci, 1, m_dynamicFilter)) {
      m_errorMessage = "Incorrect number of parameters.";
      return qfalse;
   }
   return qtrue;
}

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

qbool CMongoDB::clearFilters(EXTCompInfo *)
{
   m_filters.clear();
   m_dynamicFilter.clear();
   return qtrue;
}

/*
** Open (or reuse) the mongoc_client -> database -> collection chain.
** Returns false and sets errorMessage on failure.
*/
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
   if (!m_Database || mongoc_database_get_name(m_Database) != m_database) {
      if (m_Collection) {
         mongoc_collection_destroy(m_Collection);
         m_Collection = nullptr;
      }
      if (m_Database) {
         mongoc_database_destroy(m_Database);
         m_Database = nullptr;
      }
      m_Database = mongoc_client_get_database(m_Client, m_database.c_str());
   }

   if (!m_Collection) {
      m_Collection = mongoc_database_get_collection(m_Database, m_collection.c_str());
   }

   return true;
}

bool CMongoDB::validateFind()
{
   static const std::vector<std::string> FORBIDDEN_OPERATORS = {
       "$where", "$function", "$accumulator", "$merge", "$out"
   };

   if (m_connectionString.empty())
   {
      m_errorMessage = "Connection string not defined.";
      return false;
   }
   if (m_database.empty()) {
      m_errorMessage = "Database not defined.";
      return false;
   }
   if (m_collection.empty()) {
      m_errorMessage = "Collection not defined.";
      return false;
   }

   for (const std::string &op : FORBIDDEN_OPERATORS) {
      if (m_dynamicFilter.find(op) != std::string::npos) {
         m_errorMessage = "Dynamic filter contains invalid operator (" + op + ")";
         return false;
      }
   }
   return true;
}

/*
**  Find
*/
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

   mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(m_Collection, filter, nullptr, nullptr);
   bson_destroy(filter);

   const bson_t *doc = nullptr;
   bson_error_t  err{};

   while (mongoc_cursor_next(cursor, &doc)) {
      std::unique_ptr<EXTqlist> rowList(new EXTqlist(listVlen));
      rowList->setRow(qtrue);

      fillDestList(doc, rowList.get());

      qlong numLin = retList->insertRow();
      listToEXTqlist(numLin, 1, rowList.get(), retList.get(), true);
   }

   if (mongoc_cursor_error(cursor, &err)) {
      m_errorMessage = err.message;
      mongoc_cursor_destroy(cursor);
      return qfalse;
   }

   mongoc_cursor_destroy(cursor);
   ECOsetParameterChanged(pEci, 1);
   return qtrue;
}

/*
** Build the complete filter document.
** Returns a heap-allocated bson_t; caller must bson_destroy() it.
*/
bson_t *CMongoDB::buildFilter()
{
   std::vector<bson_t *> items;

   for (const FilterItem &itm : m_filters) {
      bson_t *item = bson_new();
      if (!buildFilterItem(item, itm.fieldName, itm.oper, itm.value, itm.type)) {
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
      if (dynDoc)
         items.push_back(dynDoc);
   }

   // No filters -> return empty document.
   if (items.empty())
      return bson_new();

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

/*
** Append a single field comparison to dest.
** dest must already be initialised by the caller (bson_new() or similar).
*/
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

/*
** Append a typed value to a BSON document under the given key.
*/
bool CMongoDB::appendBsonValue(bson_t *doc, const std::string &key, const std::string &value, DataTypes type)
{
   const char *k = key.c_str();

   switch (type) {
   case dtCHARACTER:
      BSON_APPEND_UTF8(doc, k, value.c_str());
      break;
   case dtINTEGER_32:
      BSON_APPEND_INT32(doc, k, static_cast<int32_t>(std::stoi(value)));
      break;
   case dtINTEGER_64:
      BSON_APPEND_INT64(doc, k, static_cast<int64_t>(std::stoll(value)));
      break;
   case dtDOUBLE:
      BSON_APPEND_DOUBLE(doc, k, std::stod(value));
      break;
   case dtBOOLEAN:
      BSON_APPEND_BOOL(doc, k, value == "true" || value == "1");
      break;
   case dtNULL:
      BSON_APPEND_NULL(doc, k);
      break;
   case dtDATE_TIME: {
      int64_t ms;
      if (!iso8601ToMilliseconds(value, ms))
         return false;

      BSON_APPEND_DATE_TIME(doc, k, ms);
      break;
   }

   default:
      m_errorMessage = std::string("Unsupported DataType: ") + std::to_string(type);
      return false;
   }

   return true;
}

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
      /*
      const uint8_t *data = nullptr;
      uint32_t len = 0;
      bson_iter_array(&iter, &len, &data);
      bson_t arr = BSON_INITIALIZER;
      bson_init_static(&arr, data, len);
      char *json = bson_as_relaxed_extended_json(&arr, nullptr);
      std::string result(json);
      bson_free(json);
      return result;
      */
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
