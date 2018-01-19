#ifndef PATH_HXX
#define PATH_HXX

#include <string>
#include <cstddef> // std::size_t

#include <odb/core.hxx>

#pragma db object
class security_path
{
public:
	security_path(const std::string& path)
		: path_(path)
	{
	}

	const std::string&
		path() const
	{
		return path_;
	}



private:
	friend class odb::access;

	security_path() {}

#pragma db id auto
	unsigned long id_;

	std::string path_;

};


#endif // 
