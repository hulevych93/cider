// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

%include <std_string.i>
%include <std_vector.i>
%include <stdint.i>

%include <types.swig>

%inline %{
#include <iostream>

namespace cider {

template<typename Type>
struct TypedValue final {
    TypedValue(Type val) : value(val) {}

    Type value;
};

} // namespace cider
%}

%template(UChar) cider::TypedValue<unsigned char>;
%template(UInt) cider::TypedValue<unsigned int>;
%template(UShort) cider::TypedValue<unsigned short>;
%template(ULong) cider::TypedValue<unsigned long>;
%template(ULongLong) cider::TypedValue<unsigned long long>;

%template(Char) cider::TypedValue<char>;
%template(Int) cider::TypedValue<int>;
%template(Short) cider::TypedValue<short>;
%template(Long) cider::TypedValue<long>;
%template(LongLong) cider::TypedValue<long long>;

decl_integer_based(unsigned int)
decl_integer_based(unsigned long)
decl_integer_based(unsigned short)
decl_integer_based(unsigned long long)
decl_integer_based(unsigned char)

decl_integer_based(int)
decl_integer_based(signed int)
decl_integer_based(long)
decl_integer_based(signed long)
decl_integer_based(short)
decl_integer_based(signed short)
decl_integer_based(long long)
decl_integer_based(signed long long)
decl_integer_based(char)
decl_integer_based(signed char)

decl_integer_based_alias(uint8_t, unsigned char)
decl_integer_based_alias(std::uint8_t, unsigned char)

decl_integer_based_alias(uint16_t, unsigned short)
decl_integer_based_alias(std::uint16_t, unsigned short)

decl_integer_based_alias(uint32_t, unsigned int)
decl_integer_based_alias(std::uint32_t, unsigned int)

decl_integer_based_alias(uint64_t, unsigned long long)
decl_integer_based_alias(std::uint64_t, unsigned long long)

decl_integer_based_alias(size_t, unsigned long)
decl_integer_based_alias(std::size_t, unsigned long)

decl_integer_based_alias(int8_t, char)
decl_integer_based_alias(std::int8_t, char)

decl_integer_based_alias(int16_t, short)
decl_integer_based_alias(std::int16_t, short)

decl_integer_based_alias(int32_t, int)
decl_integer_based_alias(std::int32_t, int)

decl_integer_based_alias(int64_t, long long)
decl_integer_based_alias(std::int64_t, long long)

decl_string_based(const char*)

decl_string_based(string)
decl_string_based(std::string)

decl_bool_based(bool)

decl_float_based(double)
decl_float_based(float)

%rename(assignOp) *::operator =;
%rename(plusEqualOp) *::operator +=;
%rename(minusEqualOp) *::operator -=;
%rename(multiplyEqualOp) *::operator *=;
%rename(divideEqualOp) *::operator /=;
%rename(percentEqualOp) *::operator %=;
%rename(plusOp) *::operator +;
%rename(minusOp) *::operator-;
%rename(multiplyOp) *::operator *;
%rename(divideOp) *::operator /;
%rename(percentOp) *::operator %;
%rename(notOp) *::operator !;
%rename(indexIntoOp) *::operator [];
%rename(functorOp) *::operator ();
%rename(equalEqualOp) *::operator ==;
%rename(notEqualOp) *::operator !=;
%rename(lessThanOp) *::operator <;
%rename(lessThanEqualOp) *::operator <=;
%rename(greaterThanOp) *::operator >;
%rename(greaterThanEqualOp) *::operator >=;
%rename(andOp) *::operator &&;
%rename(orOp) *::operator ||;
%rename(plusPlusPrefixOp) *::operator ++;
%rename(minusMinusPrefixOp) *::operator --;

%rename(assignOp) operator =;
%rename(plusEqualOp) operator +=;
%rename(minusEqualOp) operator -=;
%rename(multiplyEqualOp) operator *=;
%rename(divideEqualOp) operator /=;
%rename(percentEqualOp) operator %=;
%rename(plusOp) operator +;
%rename(minusOp) operator -;
%rename(multiplyOp) operator *;
%rename(divideOp) operator /;
%rename(percentOp) operator %;
%rename(notOp) operator !;
%rename(indexIntoOp) operator [];
%rename(functorOp) operator ();
%rename(equalEqualOp) operator ==;
%rename(notEqualOp) operator !=;
%rename(lessThanOp) operator <;
%rename(lessThanEqualOp) operator <=;
%rename(greaterThanOp) operator >;
%rename(greaterThanEqualOp) operator >=;
%rename(andOp) operator &&;
%rename(orOp) operator ||;
%rename(plusPlusPrefixOp) operator ++;
%rename(minusMinusPrefixOp) operator --;

%rename(toBool) *::operator bool;
%rename(toFloat) *::operator float;
%rename(toDouble) *::operator double;
%rename(toLongDouble) *::operator long double;
%rename(toChar) *::operator char;
%rename(toUnsignedChar) *::operator unsigned char;
%rename(toShort) *::operator short;
%rename(toInt) *::operator int;
%rename(toLong) *::operator long;
%rename(toLongLong) *::operator long long;
%rename(toUnsignedShort) *::operator unsigned short;
%rename(toUnsignedInt) *::operator unsigned int;
%rename(toUnsignedLong) *::operator unsigned long;
%rename(toUnsignedLongLong) *::operator unsigned long long;
%rename(toString) *::operator std::string;
%rename(toConstCharString) *::operator const char *;

%include <exception.i>

%{
namespace
{
void pushError(lua_State* L, const char* message)
{
    luaL_where(L, 1);
    lua_pushstring(L, message);
    lua_concat(L, 2);
}
} // namespace
%}

%exception
{
    try
    {
        $action
    }
    catch (const std::exception& e)
    {
        pushError(L, e.what());
        SWIG_fail;
    }
    catch (...)
    {
        pushError(L, "Unknown error");
        SWIG_fail;
    }
}
