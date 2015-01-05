#include "bigNumber.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <time.h>
using std::ifstream;
using std::ofstream;

bigNumber::bigNumber()
{
	this->_size = 1;
	this->_sign = 0;
	this->_digits = new unsigned int[1];
	this->_digits[0] = 0;
}

bigNumber::bigNumber(const char* string)
										// логика функции:
										// проверить входную строку
										// представить каждые 9 символов в виде числа
										// записать числа массив коэффициентов
										// я не смогла придумать, как её упростить
{// конструктор из строки

	if (!string)
		return;

	this->_size = 0;

	int strSize = strlen(string);
	int strSign = 0;
	if (string[0] == '-')
	{
		strSize--;
		strSign = 1;
	}
	// проверка входной строки
	const char* pStr = string + strSign;
	while (*pStr)
	{
		if (*pStr < '0' || *pStr > '9')
		{
			this->_size = 0;
			_SetSize(1);
			return;
		}
		pStr++;
	}


	// количество разрядов - округление до большего целого от (длина строки) / 9
	this->_SetSize((strSize + strSign + 8) / 9); 

	// разбиваем строку на части по 9 символов
	for (int i = 0; i < (strSize + strSign) / 9; i++)
	{
		pStr -= 9;
		char splStr[10];
		memcpy(splStr, pStr, 9);
		splStr[9] = '\0'; // получили очередную строку из 9 символов
		unsigned int digitI = atol(splStr);
		this->_digits[i] = digitI;
	}

	// обрабатываем оставшиеся символы (если длина строки не кратна 9)
	char ost[10];
	memset(ost, 0, 10);
	memcpy(ost, string + strSign, pStr - string - strSign); // получили строку - необработанная часть
	if (strlen(ost) > 0)
	{
		unsigned int lastDigit = atol(ost);
		this->operator[](-1) = lastDigit;
	}

	this->_sign = strSign;
	this->_DelNeedlessZeros();
}

bigNumber::bigNumber(const bigNumber &rhv)
{
	_Copy(rhv);
}

bigNumber::bigNumber(long long int value)
{
	this->_digits = new unsigned int[3]();
	this->_size = 0;
	this->_sign = 0;
	long long int carry = value;
	if (carry < 0)
	{
		this->_sign = 1;
		carry = -carry;
	}
	do
	{
		this->_size++;
		this->_digits[this->_size - 1] = carry % BASE;
		carry = carry / BASE;
	} while (carry);
	_DelNeedlessZeros();
}

bigNumber::~bigNumber()
{
	if (this->_size) delete[] _digits;
}


char* bigNumber::GetString()
{// возвращает строку, в которой записано число в 10-ричной системе счисления
	char* strBuffer = new char[this->_size * 9 + 1 + this->_sign]();
	char* pString = strBuffer + this->_size * 9 + this->_sign; // указатель на текущую позицию для записи числа

	for (int i = 0; i < this->_size; i++)
	{
		// получаем строковое представление очередного разряда
		char splStr[10];
		sprintf(splStr, "%09u", this->_digits[i]);

		pString -= 9;
		memcpy(pString, splStr, 9);
	}

	// удаление ведущих нулей
	while (*pString == '0' && *(pString + 1))
		pString++;

	if (this->_sign)
	{
		pString--;
		*pString = '-';
	}

	char* string = new char[strlen(pString) + 1]();
	strcpy(string, pString);
	delete[] strBuffer;

	return string;
}

char* bigNumber::__str__()
{
	return GetString();
}

char* bigNumber::__repr__()
{
	return GetString();
}

bool bigNumber::GetNumberFromFile(const char* filename)
{
	ifstream Text_file(filename);
	if (Text_file.fail())
		return false;

	Text_file.seekg(0, std::ios::end);
	int SizeOfFile = Text_file.tellg();
	Text_file.seekg(0, std::ios::beg);

	char* string = new char[SizeOfFile + 1]();
	Text_file >> string;
	Text_file.close();

	*this = bigNumber(string);
	delete[] string;
	return true;
}

bool bigNumber::SaveNumberToFile(const char* filename)
{
	ofstream Result_file(filename);
	if (Result_file.fail())
		return false;

	char* string = this->GetString();
	Result_file << string;
	delete[] string;
	Result_file.close();

	return true;
}

bool bigNumber::SaveNumberInBinFile(const char* filename)
{// будем считать, что в бинарный файл необходимо записать число по основанию 256
	// то есть необходимо перейти от BASE к 256 (256 - потому что максимальное значение байта - 255)
	// для этого надо находить остатки от деления на 256
	// пока число не уменьшиться до 0

	ofstream Result_file(filename, std::ios::binary);
	if (Result_file.fail())
		return false;

	bigNumber temp = *this; // число, которое будем делить на 256
	bigNumber b256 = 256; // чтобы каждый раз не вызывать конструктор
	bigNumber b0 = (long long int)0;

	while (temp != b0)
	{
		bigNumber remainder;
		temp = _Division(temp, b256, remainder); // делим temp на 256, в remainder - остаток

		// то, что осталось в remainder - необходимо записать в файл, точнее самый младший разряд
		// ведь остаток от деления на 256 при BASE > 256 будет занимать один разряд
		unsigned char ost = remainder[0];
		Result_file.write((char*)&ost, 1);
	}

	Result_file.close();
	return true;
}

bool bigNumber::GetNumberFromBinFile(const char* filename)
{// будем считать, что в бинарном файле записаны разряды числа по модулю 256.
	// таким образом, чтобы считать число из бинарного файла необходимо каждый байт 
	// умножить на 256 ^ i, где i - позиция байта в файле и всё это сложить
	ifstream Bin_file(filename, std::ios::binary);

	if (Bin_file.fail())
		return false;

	Bin_file.seekg(0, std::ios::end);
	int SizeOfFile = Bin_file.tellg();
	Bin_file.seekg(0, std::ios::beg);

	// считываем содержимое файла
	unsigned char* fileContent = new unsigned char[SizeOfFile];
	Bin_file.read((char*)fileContent, SizeOfFile);
	Bin_file.close();

	// переведём содержимое файла к основанию BASE
	bigNumber res;
	bigNumber b256 = 1;
	for (int i = 0; i < SizeOfFile; i++)
	{
		unsigned int fi = fileContent[i];
		res = res + b256 * fi;
		b256 = b256 * 256;
	}
	*this = res;
	return true;
}

bool bigNumber::Odd()
{
    if (this->_size == 0)
		return false;

	return (this->_digits[0] & 1);
}

bigNumber bigNumber::operator=(const bigNumber& rhv)
{
	if (this->_digits == rhv._digits)
		return *this;
	if (this->_size)
		delete[] this->_digits;
	_Copy(rhv);
	return *this;
}


bigNumber bigNumber::operator+(const bigNumber& right) const
{
	return _Sum_and_Sub(*this, right);
}

bigNumber bigNumber::operator-() const
{// унарный минус
	bigNumber res(*this);
	res._sign = !res._sign;
	return res;
}

bigNumber bigNumber::operator-(const bigNumber& right) const
{
	return *this + (-right);
}

bigNumber bigNumber::operator*(const bigNumber& right) const
{
	return _Multiplication(*this, right);
}

bigNumber bigNumber::operator/(const bigNumber& right) const
{
	bigNumber rem;
	return _Division(*this, right, rem);
}

bigNumber bigNumber::operator%(const bigNumber& right) const
{
	bigNumber rem;
	_Division(*this, right, rem);
	return rem;
}

bigNumber bigNumber::operator^(const bigNumber& right) const
{// возведение *this в степень right
	bigNumber res = 1;
	bigNumber base = *this;
	for (bigNumber i = right; i > (long long int) 0; i = i - 1)
		res = res * base;
	return res;
}



bool bigNumber::operator>(const bigNumber& B)
{
	return this->_Compare(B) > 0;
}

bool bigNumber::operator>=(const bigNumber& B)
{
	return this->_Compare(B) >= 0;
}

bool bigNumber::operator<(const bigNumber& B)
{
	return this->_Compare(B) < 0;
}

bool bigNumber::operator<=(const bigNumber& B)
{
	return this->_Compare(B) <= 0;
}

bool bigNumber::operator==(const bigNumber& B)
{
	return this->_Compare(B) == 0;
}

bool bigNumber::operator!=(const bigNumber& B)
{
	return this->_Compare(B) != 0;
}


bigNumber bigNumber::operator+(const int& right) const
{
	return *this + (bigNumber)right;
}

bigNumber bigNumber::operator-(const int& right) const
{
	return *this + (bigNumber)(-right);
}

bigNumber bigNumber::operator*(const int& right) const
{
	return (*this * (bigNumber)right);
}

bigNumber bigNumber::operator/(const int& right) const
{
	bigNumber rem;
	return _Division(*this, (bigNumber)right, rem);
}

bigNumber bigNumber::operator%(const int& right) const
{
	bigNumber rem;
	_Division(*this, (bigNumber)right, rem);
	return rem;
}

bool bigNumber::operator>(const int& B)
{
	return this->_Compare((bigNumber)B) > 0;
}

bool bigNumber::operator>=(const int& B)
{
	return this->_Compare((bigNumber)B) >= 0;
}

bool bigNumber::operator<(const int& B)
{
	return this->_Compare((bigNumber)B) < 0;
}

bool bigNumber::operator<=(const int& B)
{
	return this->_Compare((bigNumber)B) <= 0;
}

bool bigNumber::operator==(const int& B)
{
	return this->_Compare((bigNumber)B) == 0;
}

bool bigNumber::operator!=(const int& B)
{
	return this->_Compare((bigNumber)B) != 0;
}


std::ostream& operator<<(std::ostream &out, bigNumber A)
{
	char* str = A.GetString();
	out << str;
	delete[] str;
	return out;
}

std::istream& operator>>(std::istream &is, bigNumber &A)
{
	char string[10000];
	is >> string;
	bigNumber res(string);
	A = res;
	return is;
}

void bigNumber::_SetSize(int size)
{	// изменяет размер числа, при этом обнуляя его. 
	if (this->_size)
		delete[] this->_digits;
	this->_size = size;
	this->_sign = 0;
	this->_digits = new unsigned int[this->_size]();
}

unsigned int & bigNumber::operator[](int i)
{
	if (i < 0)
		return this->_digits[this->_size + i];
	return this->_digits[i];
}

unsigned int bigNumber::operator[](int i) const
{
	if (i < 0)
		return this->_digits[this->_size + i];
	return this->_digits[i];
}

void bigNumber::_Copy(const bigNumber &rhv)
{
	this->_size = rhv._size;
	if (!_size)
	{
		this->_digits = new unsigned int[1];
		this->_digits[0] = 0;
		this->_sign = 0;
		return;
	}
	this->_digits = new unsigned int[_size];
	this->_sign = rhv._sign;
	memcpy(_digits, rhv._digits, _size*sizeof(unsigned int));
	return;
}

void bigNumber::_DelNeedlessZeros()
{
	while ((_size - 1) && _digits && _digits[_size - 1] == 0)
		this->_size--;
	if (this->_size == 1 && _digits[0] == 0)
		this->_sign = 0;
}

long long int bigNumber::_Compare(const bigNumber& B)
{
	// функция возвращает
	// 0 - если числа равны,
	// >0 - если this больше
	// <0 - если this меньше
	int thisSign = 1;
	if(this->_sign == 1)
		thisSign = -1;

	if(this->_sign != B._sign)
		return thisSign;

	if (this->_size > B._size)
		return thisSign;

	if (this->_size < B._size)
		return -thisSign;

	int i = this->_size - 1;

	while( this->_digits[i] == B[i] && i > 0)
	{
		i--;
	}
	return ((long long int) this->_digits[i] - (long long int)B[i])*thisSign;
}

void bigNumber::_ShiftLeft(int s)
{// сдвигает число на s разрядов вправо
	// то есть, по сути, это умножение на BASE^s
	// сдвиг на отрицательное s - деление на BASE^(-s)
	unsigned int* newDig = new unsigned int[this->_size + s]();
	for (int i = 0; i < this->_size; i++)
	{
		if (i + s >= 0)
		{
			newDig[i + s] = this->_digits[i];
		}
	}
	delete[] this->_digits;
	this->_digits = newDig;
	this->_size += s;
	_DelNeedlessZeros();
}

bigNumber bigNumber::_Sum_and_Sub(const bigNumber& left, const bigNumber& right) const
{
	bigNumber A = left, B = right; // в А будет большее по модулю число, в B - меньшее.
	A._sign = 0;
	B._sign = 0;
	if (A > B)
	{
		A._sign = left._sign;
		B._sign = right._sign;
	}
	else
	{
		A = right;
		B = left;
	}

	if (A._sign == B._sign)
	{// если числа одного знака, то просто складываем их и выставляем нужный знак

		bigNumber res;
		res._SetSize(A._size + 1);

		unsigned int carry = 0;
		// прибавляем число меньшей размерности к числу большей размерности
		for (int i = 0; i < B._size; i++)
		{
			unsigned int tmp = A[i] + B[i] + carry;
			res[i] = tmp % BASE;
			carry = tmp / BASE;
		}
		// добавляем перенос к оставшейся части более длинного числа
		for (int i = B._size; i < A._size; i++)
		{
			unsigned int tmp = A[i] + carry;
			res[i] = tmp % BASE;
			carry = tmp / BASE;
		}
		res[A._size] = carry;
		res._sign = A._sign;
		res._DelNeedlessZeros();
		return res;
	}
	else
	{// отнимаем одно от другого и выставляем нужный знак
		bigNumber res;
		res._SetSize(A._size);

		unsigned int carry = 0;
		for (int i = 0; i < B._size; i++)
		{
			int tmp = A[i] - B[i] - carry;
			carry = 0;
			if (tmp < 0)
			{
				carry = 1;
				tmp += BASE;
			}
			res[i] = tmp;
		}

		for (int i = B._size; i < A._size; i++)
		{
			int tmp = A[i] - carry;
			carry = 0;
			if (tmp < 0)
			{
				carry = 1;
				tmp += BASE;
			}
			res[i] = tmp;
		}
		res._sign = A._sign;
		res._DelNeedlessZeros();
		return res;
	}
}

bigNumber bigNumber::_Multiplication(const bigNumber A, const bigNumber B) const
{// простое умножение "столбиком"
	bigNumber res;
	res._SetSize(A._size + B._size);
	unsigned int carry = 0;
	for (int i = 0; i < B._size; i++)
	{
		carry = 0;
		for (int j = 0; j < A._size; j++)
		{
			unsigned long long int tmp = ( unsigned long long int) B[i] * ( unsigned long long int) A[j] + carry + res[i + j];
			carry = tmp / BASE;
			res[i + j] = tmp % BASE;
		}
		res[i + A._size] = carry;
	}

	res._sign = (A._sign != B._sign);
	res._DelNeedlessZeros();
	return res;
}

bigNumber bigNumber::_Division(const bigNumber& A, const bigNumber& B, bigNumber &remainder) const
{// возвращает целую часть от деления, в remainder - остаток
	remainder = A;
	remainder._sign = 0;

	bigNumber divider = B;
	divider._sign = 0;

	if (divider == bigNumber((long long int) 0))
	{
		throw DIV_ON_ZERO; 
	}

	if (remainder < divider)
	{
		remainder = A;
		return *(new bigNumber((long long int) 0));
	}

    if (divider._size == 1)
	{
		remainder._SetSize(1);
		bigNumber res;
		res._SetSize(A._size);
		unsigned int carry = 0;
		for (int i = A._size - 1; i >= 0; i--)
		{
			long long int temp = carry;
			temp *= BASE;
			temp += A[i];
			res[i] = temp / divider[0];
			carry = temp - res[i] * divider[0];
		}
		remainder._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
		remainder[0] = carry;
		remainder._DelNeedlessZeros();
		res._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
		res._DelNeedlessZeros();
		return res;
	}

	bigNumber res;
	res._SetSize(A._size - B._size + 1);

	for (int i = A._size - B._size + 1; i; i--)
	{
		long long int qGuessMax = BASE; // для того, чтобы qGuessMin могло быть равно BASE - 1
		long long int qGuessMin = 0;
		long long int qGuess = qGuessMax;

		// цикл - подбор бинарным поиском числа qGuess
		while (qGuessMax - qGuessMin > 1)
		{
			qGuess = (qGuessMax + qGuessMin) / 2;

			// получаем tmp = qGuess * divider * BASE^i;
			bigNumber tmp = divider * qGuess;
			tmp._ShiftLeft(i - 1);	// сдвигает число на (i - 1) разрядов вправо
									// то есть, по сути, это умножение на BASE^(i - 1)
			if (tmp > remainder)
				qGuessMax = qGuess;
			else
				qGuessMin = qGuess;
		}
		bigNumber tmp = divider * qGuessMin;
		tmp._ShiftLeft(i - 1); // умножение на BASE ^ (i - 1)
		remainder = remainder - tmp;
		res[i - 1] = qGuessMin;
	}

	res._sign = (A._sign != B._sign);
	remainder._sign = (A._sign != B._sign);
	remainder._DelNeedlessZeros();
	res._DelNeedlessZeros();

	return res;
}

bigNumber Pow(const bigNumber& A, const bigNumber& B, bigNumber& modulus)
{// возведение A в степень B по модулю modulus
	if (modulus <= (long long int) 0)
		return A ^ B;

    bigNumber a(A % modulus), b(B), result(1);

	while (b != 0)
	{
		if (b.Odd())
			result = (result * a) % modulus;
		a = (a * a) % modulus;
		b = b / 2;
	}

	return result;
}


// генерация случайного числа заданной длинны 
// косяк в том, что всё сделано по основанию 1000000000,
// поэтому нельзя просто так взять и сгенерировать число определённой
// битовой длинны. Поэтому будет немножечко вранья.
// итоговое число будет "состоять" из случайных:
// на каждой позиции будет случайное число от 0 до 1000000000
// длина одного разряда 4 байта, поэтому будет сгенерировано
// bitLen/32 случайных чисел
bigNumber GenerateRandomLen(int bitLen)
{
    bigNumber res(1);
    delete[] res._digits;
    res._size = bitLen / (4 * 8);
    res._sign = 0;
    res._digits = new unsigned int[res._size];

    for (int i = 0; i < res._size; i++)
    {
        res._digits[i] = rand() % 1000000000;
        if (i == res._size - 1)
        {// чтобы не было ведущего 0
            res._digits[i] = rand() % (1000000000 - 1) + 1;
        }
    }
    return res;
}

// генерация случайного большого числа, не превосходящего заданное
bigNumber GenerateRandomMax(bigNumber max)
{
    bigNumber res(1);
    do
    {
        delete[] res._digits;
        res._size = max._size;
        res._sign = 0;
        res._digits = new unsigned int[res._size];

        for (int i = 0; i < res._size; i++)
        {
            res._digits[i] = rand() % 1000000000;
            if (i == res._size - 1)
            {// меньше шансов, что будет превосходить max
                res._digits[i] = rand() % (max._digits[i] + 1);
            }
        }
    } while (res > max);
    
    res._DelNeedlessZeros();
    return res;
}

void initRandom()
{
    srand(time(NULL));
}