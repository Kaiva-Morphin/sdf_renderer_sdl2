#include <iostream>
#include <stdlib.h>
class Vector
{
protected:
int *v;
int len;
public:
//==========Конструкторы объектов класса======//
// Конструктор
Vector(int N=0) : v(0), len(0)
{ if (N>0) v = new int[N];
 if (v) len = N;
}
// Конструктор копирования – конструктор, создающий объект (типа vector)
// по уже имеющемуся объекту того же класса
Vector(const Vector & temp) : v(0), len(0)
{ if (temp.len>0) v=new int [temp.len];
 if ( v ) {
len=temp.len;
for (int i=0;i<len;i++) {v[i]=temp.v[i];}

 }
}
//=============Размерность==========//
int size() const { return len; }
//=============Перегрузка []==========//
// для использования в левой части присваивания:
int & operator [] (int index)
{ return v[index]; }
// для использования в правой части присваивания:
int operator [] (int index) const
{ return v[index]; }
//=============Деструктор============//
// Определение деструктора ~имя_класса () {тело деструктора}
~ Vector() { delete [] v; }
};
//...
