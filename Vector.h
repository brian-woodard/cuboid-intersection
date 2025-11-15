
#ifndef VECTOR__
#define VECTOR__

#include <math.h>

class C_vector
{
public:
    double data[3];

    C_vector(void) {data[0]=0.0; data[1]=0.0; data[2]=0.0;}
    C_vector(double f)  {data[0]=f; data[1]=f; data[2]=f;}
    C_vector(double a, double b, double c)
    {data[0]=a; data[1]=b; data[2]=c;}

    void get();
    void put();
    double x() const {return data[0];}
    double y() const {return data[1];}
    double z() const {return data[2];}

    void set_x(double f) {data[0] = f;}
    void set_y(double f) {data[1] = f;}
    void set_z(double f) {data[2] = f;}

    C_vector operator=(double);
    //    C_vector operator=(C_vector&);

    void operator+=(const C_vector&);
    void operator-=(const C_vector&);

    void operator*=(double);
    void operator/=(double);

    friend C_vector operator+(const C_vector&, const C_vector&);
    friend C_vector operator-(const C_vector&, const C_vector&);

    friend C_vector operator*(double, const C_vector&);
    friend C_vector operator*(const C_vector&, double);


    friend C_vector operator/(const C_vector&, double);

    friend double operator*(const C_vector&, const C_vector&); // dot product

    friend C_vector cross(const C_vector&, const C_vector&);  // cross product
    friend double abs(const C_vector& a) {return sqrt(a*a);}
    friend double sum2(const C_vector& a) {return a*a;}

    friend C_vector unit(const C_vector& a) {return a/abs(a);}

    friend double angle(const C_vector&, const C_vector&);

};

#endif

