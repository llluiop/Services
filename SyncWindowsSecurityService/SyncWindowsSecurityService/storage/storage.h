#ifndef __STORAGE_H__
#define __STORAGE_H__
#include "../Logger/BoostLog.h"
#include "../driver/strategy.h"
#include <vector>
#include <memory>
//#include <odb/sqlite/database.hxx>

using namespace std;


class Storage
{
public:
	class DBStorage
	{
	public:
		DBStorage(const char* name);

		bool save_path(const vector<string>& monitorPaths);
		bool save_whitelist(const vector<SecurityStrategy::WhiteProcess>& whiteProcess);

		bool load_path(vector<string>& monitorPaths);
		bool load_whitelist(vector<SecurityStrategy::WhiteProcess>& whiteProcess);

		bool is_avaliable() { return true; }

	private:
		//unique_ptr<sqlite3_db> db_;
	};


	static bool SavePath(const vector<string>& monitorPaths);
	static bool SaveWhiteProcessList(const vector<SecurityStrategy::WhiteProcess>& whiteProcess);
	static bool LoadPath(vector<string>& monitorPaths);
	static bool LoadWhiteProcessList(vector<SecurityStrategy::WhiteProcess>& whiteProcess);
private:
	static DBStorage storage_;
};

#endif