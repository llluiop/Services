// file      : hello/person.hxx
// copyright : not copyrighted - public domain

#ifndef WHITELIST_HXX
#define WHITELIST_HXX

#include <string>
#include <cstddef> // std::size_t

#include <odb/core.hxx>

#pragma db object
class security_whitelist
{
public:
	security_whitelist(const std::string& name,
		const std::string& version,
		const std::string& md5)
		: name_(name), version_(version_), md5_(md5)
	{
	}

	const std::string&
		name() const
	{
		return name_;
	}

	const std::string&
		version() const
	{
		return version_;
	}

	const std::string&
		md5() const
	{
		return md5_;
	}




private:
	friend class odb::access;

	security_whitelist() {}

#pragma db id auto
	unsigned long id_;

	std::string name_;
	std::string version_;
	std::string md5_;
};



#endif // PERSON_HXX
