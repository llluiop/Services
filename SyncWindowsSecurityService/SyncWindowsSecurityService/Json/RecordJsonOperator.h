#ifndef RecordJsonOperator_h__
#define RecordJsonOperator_h__

#include <string>
#include "RecordDefine.h"


class RecordJsonOperator{
public:
	static bool Record2JsonStr(const Record &record, std::string &jsonStr);
	static bool JsonStr2Record(const std::string &jsonStr, Record &record);

	static bool Records2JsonStr(const Records &records, std::string &jsonStr);
	static bool JsonStr2Records(const std::string &jsonStr, Records &records);

	static void RecordsFilter(const Records &records, Records &recordsAfterFilter);

};


#endif // RecordJsonOperator_h__
