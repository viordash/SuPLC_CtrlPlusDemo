#pragma once

#include <stdint.h>

#define INITIAL_VERSION 0

typedef int (*TGetSizeOfCurrentData)();
typedef void (*TDataMigrateAction)(void *pCurr, void *pPrev);

typedef struct {
	uint32_t Version;
	TDataMigrateAction MigrateUp;
	TDataMigrateAction MigrateDown;
	TGetSizeOfCurrentData GetSizeOfCurrentData;
} TDataMigrate, *PTDataMigrate;

typedef struct {
	const TDataMigrate *Items;
	int Count;
} TDataMigrateItems;

typedef enum { MigrateRes_Error, MigrateRes_Migrate, MigrateRes_Skipped } TMigrateResult;

typedef bool (*TOnMigrateDataItem)(void *parent, void *pItem);

class MigrateData {
  public:
	static TMigrateResult Run(uint32_t storedVersion, uint32_t currentVersion, const TDataMigrateItems *pMigrateData, void *pStoredData, uint32_t storedSize,
							  void *parent, TOnMigrateDataItem onMigrateDataItem);

	static PTDataMigrate Find(uint32_t version, const TDataMigrateItems *pMigrateData);

  protected:
	static bool MigrateUp(PTDataMigrate pStored, void *pStoredData, PTDataMigrate pCurrent, void *pCurrentData);
	static bool MigrateDown(PTDataMigrate pStored, void *pStoredData, PTDataMigrate pCurrent, void *pCurrentData);
};

#define STRING_FIELD_MIGR_UP(field)                                                                                                                            \
	{ strncpy(pCurrSettings->field, pPrevSettings->field, sizeof(pCurrSettings->field) - 1); }

#define MEM_FIELD_MIGR_UP(field)                                                                                                                               \
	{ memcpy(&pCurrSettings->field, &pPrevSettings->field, sizeof(pCurrSettings->field)); }

#define FIELD_MIGR_UP(field)                                                                                                                                   \
	{ pCurrSettings->field = pPrevSettings->field; }

#define STRING_FIELD_MIGR_DOWN(field)                                                                                                                          \
	{ strncpy(pPrevSettings->field, pCurrSettings->field, sizeof(pPrevSettings->field) - 1); }

#define MEM_FIELD_MIGR_DOWN(field)                                                                                                                             \
	{ memcpy(&pPrevSettings->field, &pCurrSettings->field, sizeof(pPrevSettings->field)); }

#define FIELD_MIGR_DOWN(field)                                                                                                                                 \
	{ pPrevSettings->field = pCurrSettings->field; }
