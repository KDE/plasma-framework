
inputPort MathService {
    Location: "local"
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

