#include "storage.h"
//#include <odb/database.hxx>
//#include <odb/transaction.hxx>
//#include <odb/connection.hxx>
//#include <odb/schema-catalog.hxx>
//
//
//#include "path.hxx"
//#include "path-odb.hxx"
//#include "whitelist.hxx"
//#include "whitelist-odb.hxx"
#include "../Logger/BoostLog.h"

using namespace std;
//using namespace odb::core;


const char* db = "c:\\syncwindowssecurityservice.db";

bool Storage::SavePath(const vector<string>& monitorPaths)
{
	if (!storage_.is_avaliable())
	{
		SLOG_INFO << "open db failed";
		return false;
	}

	return storage_.save_path(monitorPaths);
}

bool Storage::SaveWhiteProcessList(const vector<SecurityStrategy::WhiteProcess>& whiteProcess)
{
	if (!storage_.is_avaliable())
	{
		SLOG_INFO << "open db failed";
		return false;
	}

	return storage_.save_whitelist(whiteProcess);
}

bool Storage::LoadPath(vector<string>& monitorPaths)
{
	if (!storage_.is_avaliable())
	{
		SLOG_INFO << "open db failed";
		return false;
	}

	return storage_.load_path(monitorPaths);
}

bool Storage::LoadWhiteProcessList(vector<SecurityStrategy::WhiteProcess>& whiteProcess)
{
	if (!storage_.is_avaliable())
	{
		SLOG_INFO << "open db failed";
		return false;
	}

	return storage_.load_whitelist(whiteProcess);
}

Storage::DBStorage Storage::storage_(db);

Storage::DBStorage::DBStorage(const char* name)
{
	//db_.reset(new sqlite3_db(name, SQLITE_OPEN_READWRITE));
}


bool Storage::DBStorage::save_path(const std::vector<std::string>& monitorPaths)
{
	//try
	//{
	//	transaction t(db_->begin());
	//	for (auto p : monitorPaths)
	//	{
	//		security_path path(p);
	//		db_->persist(path);
	//	}

	//	t.commit();
	//}
	//catch (const odb::exception& e)
	//{
	//	SLOG_INFO << e.what();
	//	return false;
	//}

	return true;
}

bool Storage::DBStorage::save_whitelist(const std::vector<SecurityStrategy::WhiteProcess>& whiteProcess)
{
	//try
	//{
	//	transaction t(db_->begin());
	//	for (auto p : whiteProcess)
	//	{
	//		security_whitelist list(p.name_, "", p.md5_);
	//		db_->persist(list);
	//	}

	//	t.commit();
	//}
	//catch (const odb::exception& e)
	//{
	//	SLOG_INFO << e.what();
	//	return false;
	//}

	return true;
}

bool Storage::DBStorage::load_path(vector<string>& monitorPaths)
{
	//try
	//{

	//	typedef odb::query<security_path> query;
	//	typedef odb::result<security_path> result;


	//	transaction t(db_->begin());

	//	result r(db_->query<security_path>());
	//	for (result::iterator i(r.begin()); i != r.end(); ++i)
	//	{
	//		monitorPaths.push_back(i->path());
	//	}

	//	t.commit();
	//}
	//catch (const odb::exception& e)
	//{
	//	SLOG_INFO << e.what();
	//	return false;
	//}

	return true;
}

bool Storage::DBStorage::load_whitelist(vector<SecurityStrategy::WhiteProcess>& whiteProcess)
{
	//try
	//{
	//	typedef odb::query<security_whitelist> query;
	//	typedef odb::result<security_whitelist> result;


	//	transaction t(db_->begin());

	//	result r(db_->query<security_whitelist>());
	//	for (result::iterator i(r.begin()); i != r.end(); ++i)
	//	{
	//		SecurityStrategy::WhiteProcess process{ i->name(), i->md5() };
	//		whiteProcess.push_back(process);
	//	}

	//	t.commit();
	//}
	//catch (const odb::exception& e)
	//{
	//	SLOG_INFO << e.what();
	//	return false;
	//}

	return true;
}
