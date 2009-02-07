
inputPort MathService {
    Location: "socket://localhost:11000"
    Protocol: sodep
    RequestResponse:
        twice
}

main
{
    twice(number)(result) {
        result = number * 2
    }
}

