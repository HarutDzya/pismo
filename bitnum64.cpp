#include "bitnum64.h"
#include <stdexcept>

Number::Number(const unsigned long long& num): num_(num)
{
}

const Number Number::operator&(const Number& num) const
{
	return Number(num_ & num.num_);
}

const Number Number::operator|(const Number& num) const
{
	return Number(num_ | num.num_);
}

const Number Number::operator^(const Number& num) const
{
	return Number(num_ ^ num.num_);
}

const Number Number::operator~() const
{
	return Number(~num_);
}

const Number Number::operator>>(int shift) const
{
	if (((num_ >> shift) << shift) == num_)
		return Number(num_ >> shift);
	else
		throw std::invalid_argument("Shifting removes one bits");
}

const Number Number::operator<<(int shift) const
{
	if (((num_ << shift) >> shift) == num_)
		return Number(num_ << shift);
	else
		throw std::invalid_argument("Shifting removes one bits");
}	

bool Number::operator==(const Number& num) const
{
	return num_ == num.num_;
}

const std::string Number::to_string()
{
	std::string result;
	result.resize(64+8,'\n');
	unsigned long long tmp = num_;
	
	for (int i = 63; i>=0; i--)
		{
			result[i+i/8]=(tmp&1);
			tmp>>=1;
		}
	return result;
}	
