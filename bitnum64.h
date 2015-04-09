/*bitnum64.h */

#ifndef BITNUM64_H
#define BITNUM64_H

#include <string>

//This is the wrapper class for 64 bit number


class Number
{
	public:
		Number(const unsigned long long& num);


		const std::string  to_string();
		const Number operator&(const Number& num) const;
		const Number operator|(const Number& num) const;
		const Number operator^(const Number& num) const;
		const Number operator>>(int shift) const;
		const Number operator<<(int shift) const;
		const Number operator~() const;
		bool operator==(const Number& num) const;

	private:
		unsigned long long num_;
}; 

#endif
