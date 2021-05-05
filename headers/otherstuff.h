#ifndef other_stuff_h
#define other_stuff_h

std::string numberToString (const Uint32 &number);

std::string numberToString (const Uint32 &number)
{
    std::string s;
    if (number == 0) return "0";
    int number1 = number;
    while (number1 != 0)
    {
        int tmp = number1 % 10;
        s = char (tmp + 48) + s;
        number1 /= 10;
    }
    return s;
}

#endif // other_stuff_h
