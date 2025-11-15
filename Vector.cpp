
#include <stdio.h>
#include "Vector.h"

// get a C_vector from the input stream
void C_vector::get()
{
 //   for (int i=0; i<3; i++)
//        cin>> data[i];
//        scanf("%f",data[i]);
}

// put a C_vector to the output stream
void C_vector::put()
{
    for (int i=0; i<3; i++)
//        cout<< data[i] << "  ";
//    cout<< '\n';

	printf(" %f \n", data[i]);

}

// assign a scalar to a C_vector
C_vector C_vector::operator=(double s)
{
    for (int i=0; i<3; i++)
        data[i]=s;
    return *this;
}

// assign a C_vector to a C_vector
//C_vector C_vector::operator=(C_vector& s)
//{
//  for (int i=0; i<3; i++)
//    data[i]=s.data[i];
//  return *this;
//}

// add two C_vectors
void C_vector::operator+=(const C_vector& a)
{
    for (int i=0; i<3; i++)
        data[i] += a.data[i];
}

// add two C_vectors
C_vector operator+(const C_vector& a, const C_vector& b)
{
    C_vector temp = a;
    temp+=b;

    return temp;
}

// subtract two C_vectors
void C_vector::operator-=(const C_vector& a)
{
    for (int i=0; i<3; i++)
        data[i] -= a.data[i];
}

// subtract two C_vectors
C_vector operator-(const C_vector& a, const C_vector& b)
{
    C_vector temp = a;
    temp-=b;

    return temp;
}

// multiply a C_vector by a scalar
void C_vector::operator*=(double s)
{
    for (int i=0; i<3; i++)
        data[i] *= s;
}

// multiply a C_vector by a scalar
C_vector operator*(double s, const C_vector& a)
{
    C_vector temp=a;

    for (int i=0; i<3; i++)
        temp.data[i] *= s;

    return temp;
}

// multiply a C_vector by a scalar
C_vector operator*(const C_vector& a, double s)
{
    return s*a;
}

// divide a C_vector by a scalar
void C_vector::operator/=(double s)
{
    int i;

    if (s==0.0)
//        cout<<"divide by zero error"<<endl;
		printf("divide by zero error \n");
    else
        for (i=0; i<3; i++)
            data[i] /= s;
}

// divide a C_vector by a scalar
C_vector operator/(const C_vector& a, double s)
{
    C_vector temp=a;
    temp /= s;
    return temp;
}

// C_vector dot product
double operator*(const C_vector& a, const C_vector& b)
{
    double sum=0.0;

    for (int i=0; i<3; i++)
        sum+= a.data[i]*b.data[i];

    return sum;
}

// C_vector cross product
C_vector cross(const C_vector& a, const C_vector &b)
{
    C_vector temp;

    temp.data[0] = a.data[1]*b.data[2] - a.data[2]*b.data[1];
    temp.data[1] = a.data[2]*b.data[0] - a.data[0]*b.data[2];
    temp.data[2] = a.data[0]*b.data[1] - a.data[1]*b.data[0];

    return temp;
}

//
//  Angle between to vectors  cos(angle) =  dot product over magnitude of vectors mulitplied
//
double angle (const C_vector &a, const C_vector &b)
{
    double CosAngle;

    CosAngle = (a*b)/( abs(a) * abs(b) );

    return acos(CosAngle);
}
