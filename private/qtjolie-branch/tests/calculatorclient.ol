include "console.iol"

interface CalculatorInterface {
RequestResponse:
	add
}


outputPort Calculator {
Location: "socket://localhost:9001/!/Calculator"
Protocol: soap { .debug = 1 }
Interfaces: CalculatorInterface
}

main
{
	request.x = 2;
	request.y = 3;
	add@Calculator( request )( response );
	println@Console( response )()
}
