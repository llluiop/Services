#ifndef Record_h__
#define Record_h__


#include <Windows.h>
#include <string>
#include <vector>


class Record{
public:
	Record():path_(""), reason_(""), action_(0), lastUSN_(0){}
	Record(const std::string &path, const std::string &reason, int action, LONGLONG lastUSN)
	{
		path_ = path;
		reason_ = reason;
		action_ = action;
		lastUSN_ = lastUSN;
	}

	const std::string &Path() const{return path_;}
	const std::string &Reason() const{return reason_;}
	int Action() const{return action_;}
	LONGLONG LastUSN() const{return lastUSN_;}

	void SetPath(const std::string &path){path_ = path;}
	void SetReason(const std::string &reason){reason_ = reason;}
	void SetAction(int action){action_ = action;}
	void SetLastUSN(LONGLONG lastUSN){lastUSN_ = lastUSN;}

private:
	std::string path_;
	std::string reason_;
	int action_;
	LONGLONG lastUSN_;

};


class Records{
public:
	Records(){}

	void Insert(const Record &record)
	{
		records_.push_back(record);	
	}

	void Clear(){records_.clear();}
	const std::vector<Record> &GetRecords() const{return records_;}
	bool Empty() const{return records_.empty();}
	unsigned int Size() const{return records_.size();}

private:
	std::vector<Record> records_;

};





#endif // Record_h__
