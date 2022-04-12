#include <csetjmp>
#include <iostream>
#include "exception.h"



class Dividend: public companion_item_t
{
public:
	Dividend(int dividend) : dividend(dividend) {}
	~Dividend()
	{
		std::cout << "~Dividend\n";
		//THROW(EXCEPTION);
	}
	int operator()(int divider)
	{
		if (divider == 0) THROW(DIVIDE_OR_MOD_BY_ZERO);
		return dividend / divider;
	}
private:
	int dividend;
};

int foo(int a, int b)
{
	TRY
	{
		Dividend d(a);
		return d(b);
	}
	CATCH_ALL
	{
		std::cout << "rethrow\n";
		RETHROW;
	}
}


int main()
{
	int a = 10;
	int b = 0;

	TRY
	{
		foo(a, b);
	}
	CATCH(DIVIDE_OR_MOD_BY_ZERO)
	{
		std::cout << "error caught\n";
	}

	return 0;
}