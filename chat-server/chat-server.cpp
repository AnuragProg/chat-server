#include <iostream>
#include "server.hpp";



int main(){
	const int PORT = 3000;
	ChatServer cs(PORT);
	cs.run();
}
