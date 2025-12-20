#ifndef HELLO_H
#define HELLO_H

#pragma once
#include<iostream>

class Hello {
public:
	Hello(int _a);
	~Hello();
	void print() {
		std::cout << "a = " << a << std::endl;
	}
private:
	int a;
};


#endif // !HELLO_H
