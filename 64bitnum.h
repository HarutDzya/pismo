/*64bitnum.h */
#ifndef 64BITNUM_H
#define 64BITNUM_H

//This is the wrapper class for 64 bit number


class Number
{
	public:
		Number(unsigned long long num);


		void show();
		Number &operator&(const Number &num) const;
		Number &operator|(const Number &num) const;
		Number &operator^(const Number &num) const;
		Number &operator>>(int shift) const;
		Number &operator<<(int shift) const;
		Number &operator~() const;


	private:
		unsigned long long num;
}; 

