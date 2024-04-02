#include "arbitrary_bignum.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main() {
	std::cout << "Calculator application thingy" << std::endl;

	// MENU
	// Add
	//
	// Subtract
	//
	// Multiply
	//
	// Divide
	//
	// Modulo
	//
	// Power
	//
	// Factorial
	std::string buff;	
	bool running = true;
	while(running) {
		std::cout << "Menu:\n1) Add\n2) Subtract\n3) Multiply\n4) Divide\n5) Modulo\n6) Power\n7) Factorial\n8) Quit" << std::endl;
		std::getline(std::cin,buff);
		int option = std::atoi(buff.c_str());
		if(option >= 8) {
			break;
		}

		switch(option) {
			case 1: {
				std::cout << "Input First Number: ";
				std::getline(std::cin, buff);
				auto result = std::atoll(buff.c_str());
				ArbitraryBigNum first = result;
				std::cout << "Input Second Number: ";
				std::getline(std::cin, buff);
				result = std::atoll(buff.c_str());
				ArbitraryBigNum second = result;
				std::cout << first << " + " << second << " = " << (first+second) << std::endl;
				break;
			}
			case 2: {
				std::cout << "Input First Number: ";
				std::getline(std::cin, buff);
				auto result = std::atoll(buff.c_str());
				ArbitraryBigNum first = result;
				std::cout << "Input Second Number: ";
				std::getline(std::cin, buff);
				result = std::atoll(buff.c_str());
				ArbitraryBigNum second = result;
				std::cout << first << " - " << second << " = " << (first-second) << std::endl;
				break;
			}
			case 3: {
				std::cout << "Input First Number: ";
				std::getline(std::cin, buff);
				auto result = std::atoll(buff.c_str());
				ArbitraryBigNum first = result;
				std::cout << "Input Second Number: ";
				std::getline(std::cin, buff);
				result = std::atoll(buff.c_str());
				ArbitraryBigNum second = result;
				std::cout << first << " * " << second << " = " << (first*second) << std::endl;
				break;
			}
			case 4: {
				std::cout << "Input First Number: ";
				std::getline(std::cin, buff);
				auto result = std::atoll(buff.c_str());
				ArbitraryBigNum first = result;
				std::cout << "Input Second Number: ";
				std::getline(std::cin, buff);
				result = std::atoll(buff.c_str());
				ArbitraryBigNum second = result;
				std::cout << first << " / " << second << " = " << (first/second) << std::endl;
				break;
			}
			case 5: {
				std::cout << "Input First Number: ";
				std::getline(std::cin, buff);
				auto result = std::atoll(buff.c_str());
				ArbitraryBigNum first = result;
				std::cout << "Input Second Number: ";
				std::getline(std::cin, buff);
				result = std::atoll(buff.c_str());
				ArbitraryBigNum second = result;
				std::cout << first << " % " << second << " = " << (first%second) << std::endl;
				break;
			}
			case 6: {
				std::cout << "Input First Number: ";
				std::getline(std::cin, buff);
				auto result = std::atoll(buff.c_str());
				ArbitraryBigNum first = result;
				std::cout << "Input Second Number: ";
				std::getline(std::cin, buff);
				std::size_t second = std::atoll(buff.c_str());
				std::cout << first << " ^ " << second << " = " << pow(first, second) << std::endl;
				break;
			}
			case 7: {
				std::cout << "Input number: ";
				std::getline(std::cin, buff);
				std::uint32_t value = std::atoi(buff.c_str());
				ArbitraryBigNum res = value;
				std::cout << value <<"! = ";
				value--;
				while(value > 0) {
					res *= value;
					value--;
				}
				std::cout << res << std::endl;
				break;
			}
			default:
				running = false;
				break;
		}
	}

	return 0;
}
