include "console.iol"

inputPort PrinterService {
    Location: "local"
    Protocol: sodep
    RequestResponse:
        printInput
}

main
{
    printInput(input)(result) {
        println@Console(input)();
        result = "success"
    }
}

