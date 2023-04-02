/// autogenerated analytical inverse kinematics code from ikfast program part of OpenRAVE
/// \author Rosen Diankov
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///     http://www.apache.org/licenses/LICENSE-2.0
/// 
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// ikfast version 0x1000004b generated on 2023-03-30 22:33:10.430636
/// Generated using solver transform6d
/// To compile with gcc:
///     gcc -lstdc++ ik.cpp
/// To compile without any main function as a shared object (might need -llapack):
///     gcc -fPIC -lstdc++ -DIKFAST_NO_MAIN -DIKFAST_CLIBRARY -shared -Wl,-soname,libik.so -o libik.so ik.cpp
#define IKFAST_HAS_LIBRARY
#include "ikfast.h" // found inside share/openrave-X.Y/python/ikfast.h
using namespace ikfast;

// check if the included ikfast version matches what this file was compiled with
#define IKFAST_COMPILE_ASSERT(x) extern int __dummy[(int)x]
IKFAST_COMPILE_ASSERT(IKFAST_VERSION==0x1000004b);

#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>
#include <complex>

#ifndef IKFAST_ASSERT
#include <stdexcept>
#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#endif
#endif

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __func__
#endif

#define IKFAST_ASSERT(b) { if( !(b) ) { std::stringstream ss; ss << "ikfast exception: " << __FILE__ << ":" << __LINE__ << ": " <<__PRETTY_FUNCTION__ << ": Assertion '" << #b << "' failed"; throw std::runtime_error(ss.str()); } }

#endif

#if defined(_MSC_VER)
#define IKFAST_ALIGNED16(x) __declspec(align(16)) x
#else
#define IKFAST_ALIGNED16(x) x __attribute((aligned(16)))
#endif

#define IK2PI  ((IkReal)6.28318530717959)
#define IKPI  ((IkReal)3.14159265358979)
#define IKPI_2  ((IkReal)1.57079632679490)

#ifdef _MSC_VER
#ifndef isnan
#define isnan _isnan
#endif
#ifndef isinf
#define isinf _isinf
#endif
//#ifndef isfinite
//#define isfinite _isfinite
//#endif
#endif // _MSC_VER

// lapack routines
extern "C" {
  void dgetrf_ (const int* m, const int* n, double* a, const int* lda, int* ipiv, int* info);
  void zgetrf_ (const int* m, const int* n, std::complex<double>* a, const int* lda, int* ipiv, int* info);
  void dgetri_(const int* n, const double* a, const int* lda, int* ipiv, double* work, const int* lwork, int* info);
  void dgesv_ (const int* n, const int* nrhs, double* a, const int* lda, int* ipiv, double* b, const int* ldb, int* info);
  void dgetrs_(const char *trans, const int *n, const int *nrhs, double *a, const int *lda, int *ipiv, double *b, const int *ldb, int *info);
  void dgeev_(const char *jobvl, const char *jobvr, const int *n, double *a, const int *lda, double *wr, double *wi,double *vl, const int *ldvl, double *vr, const int *ldvr, double *work, const int *lwork, int *info);
}

using namespace std; // necessary to get std math routines

#ifdef IKFAST_NAMESPACE
namespace IKFAST_NAMESPACE {
#endif

inline float IKabs(float f) { return fabsf(f); }
inline double IKabs(double f) { return fabs(f); }

inline float IKsqr(float f) { return f*f; }
inline double IKsqr(double f) { return f*f; }

inline float IKlog(float f) { return logf(f); }
inline double IKlog(double f) { return log(f); }

// allows asin and acos to exceed 1. has to be smaller than thresholds used for branch conds and evaluation
#ifndef IKFAST_SINCOS_THRESH
#define IKFAST_SINCOS_THRESH ((IkReal)1e-7)
#endif

// used to check input to atan2 for degenerate cases. has to be smaller than thresholds used for branch conds and evaluation
#ifndef IKFAST_ATAN2_MAGTHRESH
#define IKFAST_ATAN2_MAGTHRESH ((IkReal)1e-7)
#endif

// minimum distance of separate solutions
#ifndef IKFAST_SOLUTION_THRESH
#define IKFAST_SOLUTION_THRESH ((IkReal)1e-6)
#endif

// there are checkpoints in ikfast that are evaluated to make sure they are 0. This threshold speicfies by how much they can deviate
#ifndef IKFAST_EVALCOND_THRESH
#define IKFAST_EVALCOND_THRESH ((IkReal)0.03) // 5D IK has some crazy degenerate cases, but can rely on jacobian refinment to make better, just need good starting point
#endif


inline float IKasin(float f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return float(-IKPI_2);
else if( f >= 1 ) return float(IKPI_2);
return asinf(f);
}
inline double IKasin(double f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return -IKPI_2;
else if( f >= 1 ) return IKPI_2;
return asin(f);
}

// return positive value in [0,y)
inline float IKfmod(float x, float y)
{
    while(x < 0) {
        x += y;
    }
    return fmodf(x,y);
}

// return positive value in [0,y)
inline double IKfmod(double x, double y)
{
    while(x < 0) {
        x += y;
    }
    return fmod(x,y);
}

inline float IKacos(float f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return float(IKPI);
else if( f >= 1 ) return float(0);
return acosf(f);
}
inline double IKacos(double f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return IKPI;
else if( f >= 1 ) return 0;
return acos(f);
}
inline float IKsin(float f) { return sinf(f); }
inline double IKsin(double f) { return sin(f); }
inline float IKcos(float f) { return cosf(f); }
inline double IKcos(double f) { return cos(f); }
inline float IKtan(float f) { return tanf(f); }
inline double IKtan(double f) { return tan(f); }
inline float IKsqrt(float f) { if( f <= 0.0f ) return 0.0f; return sqrtf(f); }
inline double IKsqrt(double f) { if( f <= 0.0 ) return 0.0; return sqrt(f); }
inline float IKatan2Simple(float fy, float fx) {
    return atan2f(fy,fx);
}
inline float IKatan2(float fy, float fx) {
    if( isnan(fy) ) {
        IKFAST_ASSERT(!isnan(fx)); // if both are nan, probably wrong value will be returned
        return float(IKPI_2);
    }
    else if( isnan(fx) ) {
        return 0;
    }
    return atan2f(fy,fx);
}
inline double IKatan2Simple(double fy, double fx) {
    return atan2(fy,fx);
}
inline double IKatan2(double fy, double fx) {
    if( isnan(fy) ) {
        IKFAST_ASSERT(!isnan(fx)); // if both are nan, probably wrong value will be returned
        return IKPI_2;
    }
    else if( isnan(fx) ) {
        return 0;
    }
    return atan2(fy,fx);
}

template <typename T>
struct CheckValue
{
    T value;
    bool valid;
};

template <typename T>
inline CheckValue<T> IKatan2WithCheck(T fy, T fx, T epsilon)
{
    CheckValue<T> ret;
    ret.valid = false;
    ret.value = 0;
    if( !isnan(fy) && !isnan(fx) ) {
        if( IKabs(fy) >= IKFAST_ATAN2_MAGTHRESH || IKabs(fx) > IKFAST_ATAN2_MAGTHRESH ) {
            ret.value = IKatan2Simple(fy,fx);
            ret.valid = true;
        }
    }
    return ret;
}

inline float IKsign(float f) {
    if( f > 0 ) {
        return float(1);
    }
    else if( f < 0 ) {
        return float(-1);
    }
    return 0;
}

inline double IKsign(double f) {
    if( f > 0 ) {
        return 1.0;
    }
    else if( f < 0 ) {
        return -1.0;
    }
    return 0;
}

template <typename T>
inline CheckValue<T> IKPowWithIntegerCheck(T f, int n)
{
    CheckValue<T> ret;
    ret.valid = true;
    if( n == 0 ) {
        ret.value = 1.0;
        return ret;
    }
    else if( n == 1 )
    {
        ret.value = f;
        return ret;
    }
    else if( n < 0 )
    {
        if( f == 0 )
        {
            ret.valid = false;
            ret.value = (T)1.0e30;
            return ret;
        }
        if( n == -1 ) {
            ret.value = T(1.0)/f;
            return ret;
        }
    }

    int num = n > 0 ? n : -n;
    if( num == 2 ) {
        ret.value = f*f;
    }
    else if( num == 3 ) {
        ret.value = f*f*f;
    }
    else {
        ret.value = 1.0;
        while(num>0) {
            if( num & 1 ) {
                ret.value *= f;
            }
            num >>= 1;
            f *= f;
        }
    }
    
    if( n < 0 ) {
        ret.value = T(1.0)/ret.value;
    }
    return ret;
}

template <typename T> struct ComplexLess
{
    bool operator()(const complex<T>& lhs, const complex<T>& rhs) const
    {
        if (real(lhs) < real(rhs)) {
            return true;
        }
        if (real(lhs) > real(rhs)) {
            return false;
        }
        return imag(lhs) < imag(rhs);
    }
};

/// solves the forward kinematics equations.
/// \param pfree is an array specifying the free joints of the chain.
IKFAST_API void ComputeFk(const IkReal* j, IkReal* eetrans, IkReal* eerot) {
IkReal x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47;
x0=IKcos(j[0]);
x1=IKcos(j[1]);
x2=IKcos(j[2]);
x3=IKsin(j[1]);
x4=IKsin(j[2]);
x5=IKcos(j[3]);
x6=IKsin(j[3]);
x7=IKcos(j[5]);
x8=IKsin(j[5]);
x9=IKcos(j[4]);
x10=IKsin(j[0]);
x11=IKsin(j[4]);
x12=((0.1025)*x1);
x13=((1.0)*x6);
x14=((0.303)*x1);
x15=((1.0)*x11);
x16=((1.0)*x9);
x17=((0.093)*x5);
x18=((0.1025)*x10);
x19=((0.093)*x6);
x20=((1.0)*x1);
x21=((1.0)*x10);
x22=((1.0)*x5);
x23=(x2*x3);
x24=(x10*x2);
x25=(x10*x9);
x26=(x0*x1);
x27=(x0*x4);
x28=(x3*x4);
x29=(x10*x4);
x30=((1.0)*x27*x3);
x31=(x21*x28);
x32=(((x1*x2))+(((-1.0)*x28)));
x33=(((x1*x4))+x23);
x34=(x33*x5);
x35=((((-1.0)*x30))+((x2*x26)));
x36=((((-1.0)*x31))+((x1*x24)));
x37=((((-1.0)*x0*x23))+(((-1.0)*x20*x27)));
x38=((((-1.0)*x21*x23))+(((-1.0)*x20*x29)));
x39=(x36*x5);
x40=(x35*x5);
x41=(x37*x6);
x42=(((x32*x5))+((x6*(((((-1.0)*x23))+(((-1.0)*x20*x4)))))));
x43=((((-1.0)*x22*x33))+(((-1.0)*x13*x32)));
x44=(x40+x41);
x45=(x39+((x38*x6)));
x46=(((x37*x5))+((x6*(((((-1.0)*x0*x2*x20))+x30)))));
x47=(((x6*((x31+(((-1.0)*x20*x24))))))+((x38*x5)));
eerot[0]=(((x7*((((x44*x9))+((x10*x11))))))+((x46*x8)));
eerot[1]=(((x8*(((((-1.0)*x16*x44))+(((-1.0)*x10*x15))))))+((x46*x7)));
eerot[2]=(((x11*(((((-1.0)*x22*x35))+(((-1.0)*x13*x37))))))+x25);
IkReal x48=(x0*x2);
IkReal x49=(x27*x3);
eetrans[0]=(((x5*(((((0.1025)*x0*x23))+((x12*x27))))))+(((0.093)*x25))+((x14*x48))+(((-0.1155)*x10))+(((-0.303)*x49))+((x11*(((((-1.0)*x17*x35))+(((-1.0)*x19*x37))))))+((x6*(((((-0.1025)*x49))+((x12*x48))))))+(((0.353)*x26)));
eerot[3]=(((x7*(((((-1.0)*x0*x11))+((x45*x9))))))+((x47*x8)));
eerot[4]=(((x47*x7))+((x8*((((x0*x15))+(((-1.0)*x16*x45)))))));
eerot[5]=((((-1.0)*x0*x16))+((x11*(((((-1.0)*x22*x36))+(((-1.0)*x13*x38)))))));
eetrans[1]=((((0.353)*x1*x10))+((x11*(((((-1.0)*x17*x36))+(((-1.0)*x19*x38))))))+((x6*(((((-1.0)*x18*x28))+((x12*x24))))))+((x14*x24))+(((-0.093)*x0*x9))+((x5*((((x18*x23))+((x12*x29))))))+(((0.1155)*x0))+(((-0.303)*x10*x28)));
eerot[6]=(((x42*x8))+((x7*x9*((((x32*x6))+x34)))));
eerot[7]=(((x42*x7))+((x43*x8*x9)));
eerot[8]=(x11*x43);
eetrans[2]=((0.114)+((x5*(((((-1.0)*x12*x2))+(((0.1025)*x28))))))+((x6*(((((0.1025)*x23))+((x12*x4))))))+((x11*(((((-1.0)*x17*x33))+(((-1.0)*x19*x32))))))+(((0.353)*x3))+((x14*x4))+(((0.303)*x23)));
}

IKFAST_API int GetNumFreeParameters() { return 0; }
IKFAST_API const int* GetFreeIndices() { return NULL; }
IKFAST_API int GetNumJoints() { return 6; }

IKFAST_API int GetIkRealSize() { return sizeof(IkReal); }

IKFAST_API int GetIkType() { return 0x67000001; }

class IKSolver {
public:
IkReal j2,cj2,sj2,htj2,j2mul,j3,cj3,sj3,htj3,j3mul,j4,cj4,sj4,htj4,j4mul,j5,cj5,sj5,htj5,j5mul,j6,cj6,sj6,htj6,j6mul,j7,cj7,sj7,htj7,j7mul,new_r00,r00,rxp0_0,new_r01,r01,rxp0_1,new_r02,r02,rxp0_2,new_r10,r10,rxp1_0,new_r11,r11,rxp1_1,new_r12,r12,rxp1_2,new_r20,r20,rxp2_0,new_r21,r21,rxp2_1,new_r22,r22,rxp2_2,new_px,px,npx,new_py,py,npy,new_pz,pz,npz,pp;
unsigned char _ij2[2], _nj2,_ij3[2], _nj3,_ij4[2], _nj4,_ij5[2], _nj5,_ij6[2], _nj6,_ij7[2], _nj7;

IkReal j100, cj100, sj100;
unsigned char _ij100[2], _nj100;
bool ComputeIk(const IkReal* eetrans, const IkReal* eerot, const IkReal* pfree, IkSolutionListBase<IkReal>& solutions) {
j2=numeric_limits<IkReal>::quiet_NaN(); _ij2[0] = -1; _ij2[1] = -1; _nj2 = -1; j3=numeric_limits<IkReal>::quiet_NaN(); _ij3[0] = -1; _ij3[1] = -1; _nj3 = -1; j4=numeric_limits<IkReal>::quiet_NaN(); _ij4[0] = -1; _ij4[1] = -1; _nj4 = -1; j5=numeric_limits<IkReal>::quiet_NaN(); _ij5[0] = -1; _ij5[1] = -1; _nj5 = -1; j6=numeric_limits<IkReal>::quiet_NaN(); _ij6[0] = -1; _ij6[1] = -1; _nj6 = -1; j7=numeric_limits<IkReal>::quiet_NaN(); _ij7[0] = -1; _ij7[1] = -1; _nj7 = -1; 
for(int dummyiter = 0; dummyiter < 1; ++dummyiter) {
    solutions.Clear();
r00 = eerot[0*3+0];
r01 = eerot[0*3+1];
r02 = eerot[0*3+2];
r10 = eerot[1*3+0];
r11 = eerot[1*3+1];
r12 = eerot[1*3+2];
r20 = eerot[2*3+0];
r21 = eerot[2*3+1];
r22 = eerot[2*3+2];
px = eetrans[0]; py = eetrans[1]; pz = eetrans[2];

new_r00=r00;
new_r01=r01;
new_r02=r02;
new_px=((((-0.093)*r02))+px);
new_r10=r10;
new_r11=r11;
new_r12=r12;
new_py=((((-0.093)*r12))+py);
new_r20=r20;
new_r21=r21;
new_r22=r22;
new_pz=((-0.114)+(((-0.093)*r22))+pz);
r00 = new_r00; r01 = new_r01; r02 = new_r02; r10 = new_r10; r11 = new_r11; r12 = new_r12; r20 = new_r20; r21 = new_r21; r22 = new_r22; px = new_px; py = new_py; pz = new_pz;
IkReal x50=((1.0)*px);
IkReal x51=((1.0)*pz);
IkReal x52=((1.0)*py);
pp=((px*px)+(py*py)+(pz*pz));
npx=(((px*r00))+((py*r10))+((pz*r20)));
npy=(((px*r01))+((py*r11))+((pz*r21)));
npz=(((px*r02))+((py*r12))+((pz*r22)));
rxp0_0=((((-1.0)*r20*x52))+((pz*r10)));
rxp0_1=(((px*r20))+(((-1.0)*r00*x51)));
rxp0_2=((((-1.0)*r10*x50))+((py*r00)));
rxp1_0=((((-1.0)*r21*x52))+((pz*r11)));
rxp1_1=(((px*r21))+(((-1.0)*r01*x51)));
rxp1_2=((((-1.0)*r11*x50))+((py*r01)));
rxp2_0=(((pz*r12))+(((-1.0)*r22*x52)));
rxp2_1=(((px*r22))+(((-1.0)*r02*x51)));
rxp2_2=((((-1.0)*r12*x50))+((py*r02)));
{
IkReal j2eval[2];
j2eval[0]=((px*px)+(py*py));
j2eval[1]=((IKabs(px))+(IKabs(py)));
if( IKabs(j2eval[0]) < 0.0000010000000000  || IKabs(j2eval[1]) < 0.0000010000000000  )
{
continue; // no branches [j2]

} else
{
{
IkReal j2array[2], cj2array[2], sj2array[2];
bool j2valid[2]={false};
_nj2 = 2;
CheckValue<IkReal> x55 = IKatan2WithCheck(IkReal(((-1.0)*py)),IkReal(px),IKFAST_ATAN2_MAGTHRESH);
if(!x55.valid){
continue;
}
IkReal x53=((1.0)*(x55.value));
if((((px*px)+(py*py))) < -0.00001)
continue;
CheckValue<IkReal> x56=IKPowWithIntegerCheck(IKabs(IKsqrt(((px*px)+(py*py)))),-1);
if(!x56.valid){
continue;
}
if( (((0.1155)*(x56.value))) < -1-IKFAST_SINCOS_THRESH || (((0.1155)*(x56.value))) > 1+IKFAST_SINCOS_THRESH )
    continue;
IkReal x54=IKasin(((0.1155)*(x56.value)));
j2array[0]=((((-1.0)*x53))+(((-1.0)*x54)));
sj2array[0]=IKsin(j2array[0]);
cj2array[0]=IKcos(j2array[0]);
j2array[1]=((3.14159265358979)+x54+(((-1.0)*x53)));
sj2array[1]=IKsin(j2array[1]);
cj2array[1]=IKcos(j2array[1]);
if( j2array[0] > IKPI )
{
    j2array[0]-=IK2PI;
}
else if( j2array[0] < -IKPI )
{    j2array[0]+=IK2PI;
}
j2valid[0] = true;
if( j2array[1] > IKPI )
{
    j2array[1]-=IK2PI;
}
else if( j2array[1] < -IKPI )
{    j2array[1]+=IK2PI;
}
j2valid[1] = true;
for(int ij2 = 0; ij2 < 2; ++ij2)
{
if( !j2valid[ij2] )
{
    continue;
}
_ij2[0] = ij2; _ij2[1] = -1;
for(int iij2 = ij2+1; iij2 < 2; ++iij2)
{
if( j2valid[iij2] && IKabs(cj2array[ij2]-cj2array[iij2]) < IKFAST_SOLUTION_THRESH && IKabs(sj2array[ij2]-sj2array[iij2]) < IKFAST_SOLUTION_THRESH )
{
    j2valid[iij2]=false; _ij2[1] = iij2; break; 
}
}
j2 = j2array[ij2]; cj2 = cj2array[ij2]; sj2 = sj2array[ij2];

{
IkReal j7array[2], cj7array[2], sj7array[2];
bool j7valid[2]={false};
_nj7 = 2;
IkReal x57=cj2*cj2;
IkReal x58=r01*r01;
IkReal x59=r00*r00;
IkReal x60=(r01*sj2);
IkReal x61=(r00*sj2);
IkReal x62=(cj2*r10);
IkReal x63=(cj2*r11);
IkReal x64=((4.0)*x57);
CheckValue<IkReal> x69=IKPowWithIntegerCheck((x60+(((-1.0)*x63))),-1);
if(!x69.valid){
continue;
}
IkReal x65=x69.value;
IkReal x66=(x61*x65);
IkReal x67=((1.0)*x62*x65);
if((((((-8.0)*x60*x63))+((x64*(r11*r11)))+(((4.0)*x59))+(((4.0)*x58))+(((-1.0)*x59*x64))+((x64*(r10*r10)))+(((-1.0)*x58*x64))+(((-8.0)*x61*x62)))) < -0.00001)
continue;
IkReal x68=((0.5)*x65*(IKsqrt(((((-8.0)*x60*x63))+((x64*(r11*r11)))+(((4.0)*x59))+(((4.0)*x58))+(((-1.0)*x59*x64))+((x64*(r10*r10)))+(((-1.0)*x58*x64))+(((-8.0)*x61*x62))))));
j7array[0]=((2.0)*(atan((x66+x68+(((-1.0)*x67))))));
sj7array[0]=IKsin(j7array[0]);
cj7array[0]=IKcos(j7array[0]);
j7array[1]=((-2.0)*(atan((x67+x68+(((-1.0)*x66))))));
sj7array[1]=IKsin(j7array[1]);
cj7array[1]=IKcos(j7array[1]);
if( j7array[0] > IKPI )
{
    j7array[0]-=IK2PI;
}
else if( j7array[0] < -IKPI )
{    j7array[0]+=IK2PI;
}
j7valid[0] = true;
if( j7array[1] > IKPI )
{
    j7array[1]-=IK2PI;
}
else if( j7array[1] < -IKPI )
{    j7array[1]+=IK2PI;
}
j7valid[1] = true;
for(int ij7 = 0; ij7 < 2; ++ij7)
{
if( !j7valid[ij7] )
{
    continue;
}
_ij7[0] = ij7; _ij7[1] = -1;
for(int iij7 = ij7+1; iij7 < 2; ++iij7)
{
if( j7valid[iij7] && IKabs(cj7array[ij7]-cj7array[iij7]) < IKFAST_SOLUTION_THRESH && IKabs(sj7array[ij7]-sj7array[iij7]) < IKFAST_SOLUTION_THRESH )
{
    j7valid[iij7]=false; _ij7[1] = iij7; break; 
}
}
j7 = j7array[ij7]; cj7 = cj7array[ij7]; sj7 = sj7array[ij7];
htj7 = IKtan(j7/2);

{
IkReal j4array[2], cj4array[2], sj4array[2];
bool j4valid[2]={false};
_nj4 = 2;
cj4array[0]=((-1.02493478809637)+(((0.958311128563281)*cj7*npy))+(((0.958311128563281)*npx*sj7))+(((4.67468843201601)*pp)));
if( cj4array[0] >= -1-IKFAST_SINCOS_THRESH && cj4array[0] <= 1+IKFAST_SINCOS_THRESH )
{
    j4valid[0] = j4valid[1] = true;
    j4array[0] = IKacos(cj4array[0]);
    sj4array[0] = IKsin(j4array[0]);
    cj4array[1] = cj4array[0];
    j4array[1] = -j4array[0];
    sj4array[1] = -sj4array[0];
}
else if( isnan(cj4array[0]) )
{
    // probably any value will work
    j4valid[0] = true;
    cj4array[0] = 1; sj4array[0] = 0; j4array[0] = 0;
}
for(int ij4 = 0; ij4 < 2; ++ij4)
{
if( !j4valid[ij4] )
{
    continue;
}
_ij4[0] = ij4; _ij4[1] = -1;
for(int iij4 = ij4+1; iij4 < 2; ++iij4)
{
if( j4valid[iij4] && IKabs(cj4array[ij4]-cj4array[iij4]) < IKFAST_SOLUTION_THRESH && IKabs(sj4array[ij4]-sj4array[iij4]) < IKFAST_SOLUTION_THRESH )
{
    j4valid[iij4]=false; _ij4[1] = iij4; break; 
}
}
j4 = j4array[ij4]; cj4 = cj4array[ij4]; sj4 = sj4array[ij4];
{
IkReal evalcond[1];
IkReal x70=(cj2*cj7);
IkReal x71=((0.072365)*pp);
IkReal x72=(px*sj2);
IkReal x73=((0.14473)*npy);
IkReal x74=((0.14473)*npx*sj7);
evalcond[0]=((0.017878465836)+((cj7*x72*x73))+(((-1.0)*cj2*py*x74))+(((-1.0)*py*x70*x73))+(((-1.0)*cj7*r01*sj2*x71))+(((-1.0)*r00*sj2*sj7*x71))+(((0.017443515474)*(IKcos(j4))))+((cj2*r10*sj7*x71))+(((-0.081543)*pp))+((x72*x74))+((r11*x70*x71)));
if( IKabs(evalcond[0]) > IKFAST_EVALCOND_THRESH  )
{
continue;
}
}

{
IkReal j5eval[2];
j5eval[0]=((1.01168672108004)+cj4);
j5eval[1]=IKsign(((8873138.0)+(((8770638.0)*cj4))));
if( IKabs(j5eval[0]) < 0.0000010000000000  || IKabs(j5eval[1]) < 0.0000010000000000  )
{
continue; // no branches [j3, j5, j6]

} else
{
{
IkReal j5array[1], cj5array[1], sj5array[1];
bool j5valid[1]={false};
_nj5 = 1;
IkReal x75=((12423000.0)*sj7);
IkReal x76=(cj2*rxp0_1);
IkReal x77=(rxp0_0*sj2);
IkReal x78=((70600000.0)*pp);
IkReal x79=((14473000.0)*sj4);
IkReal x80=((14473000.0)*cj4);
IkReal x81=(cj2*cj7*rxp1_1);
IkReal x82=(sj7*x80);
IkReal x83=(cj7*rxp1_0*sj2);
CheckValue<IkReal> x84 = IKatan2WithCheck(IkReal(((-14560028.7)+((x79*x83))+(((60600000.0)*pp))+((sj7*x77*x79))+((cj4*x78))+(((-29926104.5)*cj4))+(((-1.0)*x79*x81))+(((-15102610.8)*(cj4*cj4)))+(((-1.0)*sj7*x76*x79)))),IkReal(((((-16962673.7)*sj4))+(((-1.0)*x80*x83))+(((-1.0)*x77*x82))+((x75*x76))+((x76*x82))+(((-12423000.0)*x83))+(((-15102610.8)*cj4*sj4))+((x80*x81))+(((12423000.0)*x81))+((sj4*x78))+(((-1.0)*x75*x77)))),IKFAST_ATAN2_MAGTHRESH);
if(!x84.valid){
continue;
}
CheckValue<IkReal> x85=IKPowWithIntegerCheck(IKsign(((8873138.0)+(((8770638.0)*cj4)))),-1);
if(!x85.valid){
continue;
}
j5array[0]=((-1.5707963267949)+(x84.value)+(((1.5707963267949)*(x85.value))));
sj5array[0]=IKsin(j5array[0]);
cj5array[0]=IKcos(j5array[0]);
if( j5array[0] > IKPI )
{
    j5array[0]-=IK2PI;
}
else if( j5array[0] < -IKPI )
{    j5array[0]+=IK2PI;
}
j5valid[0] = true;
for(int ij5 = 0; ij5 < 1; ++ij5)
{
if( !j5valid[ij5] )
{
    continue;
}
_ij5[0] = ij5; _ij5[1] = -1;
for(int iij5 = ij5+1; iij5 < 1; ++iij5)
{
if( j5valid[iij5] && IKabs(cj5array[ij5]-cj5array[iij5]) < IKFAST_SOLUTION_THRESH && IKabs(sj5array[ij5]-sj5array[iij5]) < IKFAST_SOLUTION_THRESH )
{
    j5valid[iij5]=false; _ij5[1] = iij5; break; 
}
}
j5 = j5array[ij5]; cj5 = cj5array[ij5]; sj5 = sj5array[ij5];
{
IkReal evalcond[4];
IkReal x86=IKsin(j5);
IkReal x87=IKcos(j5);
IkReal x88=((1.0)*cj2);
IkReal x89=(cj7*sj2);
IkReal x90=((2.0)*px);
IkReal x91=((0.353)*sj4);
IkReal x92=(pp*sj7);
IkReal x93=((0.353)*cj4);
IkReal x94=(npx*sj7);
IkReal x95=(cj7*npy);
IkReal x96=(sj4*x87);
IkReal x97=(cj4*x86);
IkReal x98=((2.0)*cj2*py);
evalcond[0]=((0.2402645)+(((-1.0)*pp))+(((0.213918)*cj4))+(((0.062115)*x86))+(((0.072365)*x96))+(((0.072365)*x97)));
evalcond[1]=((0.1025)+((x87*x91))+(((0.303)*x86))+((x86*x93))+x95+x94);
evalcond[2]=(((rxp1_0*x89))+((x87*x93))+(((0.303)*x87))+((rxp0_0*sj2*sj7))+(((-1.0)*cj7*rxp1_1*x88))+(((-1.0)*x86*x91))+(((-1.0)*rxp0_1*sj7*x88)));
evalcond[3]=((0.0236775)+(((-1.0)*r10*x88*x92))+(((-1.0)*sj2*x90*x94))+(((-1.0)*cj7*pp*r11*x88))+((r00*sj2*x92))+(((0.069993)*x86))+((pp*r01*x89))+(((0.081543)*x96))+(((0.081543)*x97))+((x95*x98))+(((-1.0)*npy*x89*x90))+((x94*x98)));
if( IKabs(evalcond[0]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[1]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[2]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[3]) > IKFAST_EVALCOND_THRESH  )
{
continue;
}
}

{
IkReal j3array[1], cj3array[1], sj3array[1];
bool j3valid[1]={false};
_nj3 = 1;
IkReal x99=((1.0)*sj5);
IkReal x100=(cj4*cj7);
IkReal x101=(rxp0_2*sj7);
IkReal x102=(cj7*rxp1_2);
IkReal x103=((8.65800865800866)*cj5);
IkReal x104=(r20*sj7);
IkReal x105=((8.65800865800866)*sj5);
IkReal x106=(cj7*r21);
IkReal x107=(sj4*x103);
IkReal x108=((1.0)*cj5*sj4);
IkReal x109=(x101*x105);
if( IKabs(((((-1.0)*x104*x108))+(((-1.0)*cj4*x101*x103))+(((-1.0)*x106*x108))+((sj4*x102*x105))+(((-1.0)*r21*x100*x99))+(((-1.0)*rxp1_2*x100*x103))+((sj4*x109))+(((-1.0)*cj4*x104*x99)))) < IKFAST_ATAN2_MAGTHRESH && IKabs((((cj5*r21*x100))+(((-1.0)*x101*x107))+(((-1.0)*sj4*x104*x99))+(((-1.0)*cj4*x109))+(((-1.0)*rxp1_2*x100*x105))+(((-1.0)*x102*x107))+(((-1.0)*sj4*x106*x99))+((cj4*cj5*x104)))) < IKFAST_ATAN2_MAGTHRESH && IKabs(IKsqr(((((-1.0)*x104*x108))+(((-1.0)*cj4*x101*x103))+(((-1.0)*x106*x108))+((sj4*x102*x105))+(((-1.0)*r21*x100*x99))+(((-1.0)*rxp1_2*x100*x103))+((sj4*x109))+(((-1.0)*cj4*x104*x99))))+IKsqr((((cj5*r21*x100))+(((-1.0)*x101*x107))+(((-1.0)*sj4*x104*x99))+(((-1.0)*cj4*x109))+(((-1.0)*rxp1_2*x100*x105))+(((-1.0)*x102*x107))+(((-1.0)*sj4*x106*x99))+((cj4*cj5*x104))))-1) <= IKFAST_SINCOS_THRESH )
    continue;
j3array[0]=IKatan2(((((-1.0)*x104*x108))+(((-1.0)*cj4*x101*x103))+(((-1.0)*x106*x108))+((sj4*x102*x105))+(((-1.0)*r21*x100*x99))+(((-1.0)*rxp1_2*x100*x103))+((sj4*x109))+(((-1.0)*cj4*x104*x99))), (((cj5*r21*x100))+(((-1.0)*x101*x107))+(((-1.0)*sj4*x104*x99))+(((-1.0)*cj4*x109))+(((-1.0)*rxp1_2*x100*x105))+(((-1.0)*x102*x107))+(((-1.0)*sj4*x106*x99))+((cj4*cj5*x104))));
sj3array[0]=IKsin(j3array[0]);
cj3array[0]=IKcos(j3array[0]);
if( j3array[0] > IKPI )
{
    j3array[0]-=IK2PI;
}
else if( j3array[0] < -IKPI )
{    j3array[0]+=IK2PI;
}
j3valid[0] = true;
for(int ij3 = 0; ij3 < 1; ++ij3)
{
if( !j3valid[ij3] )
{
    continue;
}
_ij3[0] = ij3; _ij3[1] = -1;
for(int iij3 = ij3+1; iij3 < 1; ++iij3)
{
if( j3valid[iij3] && IKabs(cj3array[ij3]-cj3array[iij3]) < IKFAST_SOLUTION_THRESH && IKabs(sj3array[ij3]-sj3array[iij3]) < IKFAST_SOLUTION_THRESH )
{
    j3valid[iij3]=false; _ij3[1] = iij3; break; 
}
}
j3 = j3array[ij3]; cj3 = cj3array[ij3]; sj3 = sj3array[ij3];
{
IkReal evalcond[8];
IkReal x110=IKsin(j3);
IkReal x111=IKcos(j3);
IkReal x112=((0.219252)*cj5);
IkReal x113=((0.029966)*cj5);
IkReal x114=((0.213918)*sj5);
IkReal x115=((0.1025)*cj5);
IkReal x116=(py*sj2);
IkReal x117=(cj2*sj7);
IkReal x118=(sj2*sj7);
IkReal x119=(cj7*pp);
IkReal x120=((0.213584)*sj5);
IkReal x121=(cj2*px);
IkReal x122=(r11*sj2);
IkReal x123=((0.035634)*sj5);
IkReal x124=((0.1155)*sj5);
IkReal x125=(cj2*cj7);
IkReal x126=((0.1025)*sj5);
IkReal x127=(r20*sj7);
IkReal x128=((0.1155)*cj5);
IkReal x129=(sj4*x111);
IkReal x130=(cj4*x110);
IkReal x131=((2.0)*npx*sj7);
IkReal x132=(cj5*x111);
IkReal x133=(cj4*x111);
IkReal x134=(sj4*x110);
IkReal x135=((2.0)*cj7*npy);
evalcond[0]=(((sj5*x129))+(((-1.0)*cj4*x132))+((sj5*x130))+x127+((cj5*x134))+((cj7*r21)));
evalcond[1]=(((cj7*rxp1_2))+(((-1.0)*x124*x134))+((x124*x133))+((rxp0_2*sj7))+((x128*x130))+((x128*x129)));
evalcond[2]=(((r10*x118))+((sj5*x133))+((r01*x125))+((cj7*x122))+((r00*x117))+(((-1.0)*sj5*x134))+((cj5*x130))+((cj5*x129)));
evalcond[3]=(((rxp1_0*x125))+((cj7*rxp1_1*sj2))+(((-1.0)*x124*x130))+(((-1.0)*x128*x134))+(((-1.0)*x124*x129))+((x128*x133))+((rxp0_0*x117))+((rxp0_1*x118)));
evalcond[4]=((((-1.0)*x115*x133))+(((0.353)*x110))+(((0.303)*x129))+(((0.303)*x130))+(((-1.0)*pz))+((x126*x130))+((x126*x129))+((x115*x134)));
evalcond[5]=((((0.353)*x111))+(((0.303)*x133))+(((-1.0)*x126*x134))+((x115*x129))+((x126*x133))+((x115*x130))+(((-1.0)*x121))+(((-1.0)*x116))+(((-0.303)*x134)));
evalcond[6]=((((-0.213918)*x132))+(((-1.0)*pz*x131))+(((-1.0)*pz*x135))+(((-1.0)*x120*x130))+(((-1.0)*x113*x134))+(((-1.0)*x112*x133))+(((-0.072365)*x110))+(((-0.062115)*x130))+(((-0.062115)*x129))+((r21*x119))+(((-1.0)*x110*x114))+((pp*x127))+((x123*x129)));
evalcond[7]=(((x112*x130))+(((-1.0)*x123*x134))+(((-1.0)*x113*x129))+(((-1.0)*x111*x114))+(((-1.0)*x120*x133))+(((0.062115)*x134))+(((0.213918)*cj5*x110))+(((-0.072365)*x111))+(((-0.062115)*x133))+((pp*r00*x117))+(((-1.0)*x116*x135))+(((-1.0)*x116*x131))+((cj2*r01*x119))+(((-1.0)*x121*x135))+((pp*r10*x118))+((x119*x122))+(((-2.0)*npx*px*x117)));
if( IKabs(evalcond[0]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[1]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[2]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[3]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[4]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[5]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[6]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[7]) > IKFAST_EVALCOND_THRESH  )
{
continue;
}
}

{
IkReal j6array[1], cj6array[1], sj6array[1];
bool j6valid[1]={false};
_nj6 = 1;
j6array[0]=0;
sj6array[0]=IKsin(j6array[0]);
cj6array[0]=IKcos(j6array[0]);
if( j6array[0] > IKPI )
{
    j6array[0]-=IK2PI;
}
else if( j6array[0] < -IKPI )
{    j6array[0]+=IK2PI;
}
j6valid[0] = true;
for(int ij6 = 0; ij6 < 1; ++ij6)
{
if( !j6valid[ij6] )
{
    continue;
}
_ij6[0] = ij6; _ij6[1] = -1;
for(int iij6 = ij6+1; iij6 < 1; ++iij6)
{
if( j6valid[iij6] && IKabs(cj6array[ij6]-cj6array[iij6]) < IKFAST_SOLUTION_THRESH && IKabs(sj6array[ij6]-sj6array[iij6]) < IKFAST_SOLUTION_THRESH )
{
    j6valid[iij6]=false; _ij6[1] = iij6; break; 
}
}
j6 = j6array[ij6]; cj6 = cj6array[ij6]; sj6 = sj6array[ij6];

{
std::vector<IkSingleDOFSolutionBase<IkReal> > vinfos(6);
vinfos[0].jointtype = 1;
vinfos[0].foffset = j2;
vinfos[0].indices[0] = _ij2[0];
vinfos[0].indices[1] = _ij2[1];
vinfos[0].maxsolutions = _nj2;
vinfos[1].jointtype = 1;
vinfos[1].foffset = j3;
vinfos[1].indices[0] = _ij3[0];
vinfos[1].indices[1] = _ij3[1];
vinfos[1].maxsolutions = _nj3;
vinfos[2].jointtype = 1;
vinfos[2].foffset = j4;
vinfos[2].indices[0] = _ij4[0];
vinfos[2].indices[1] = _ij4[1];
vinfos[2].maxsolutions = _nj4;
vinfos[3].jointtype = 1;
vinfos[3].foffset = j5;
vinfos[3].indices[0] = _ij5[0];
vinfos[3].indices[1] = _ij5[1];
vinfos[3].maxsolutions = _nj5;
vinfos[4].jointtype = 1;
vinfos[4].foffset = j6;
vinfos[4].indices[0] = _ij6[0];
vinfos[4].indices[1] = _ij6[1];
vinfos[4].maxsolutions = _nj6;
vinfos[5].jointtype = 1;
vinfos[5].foffset = j7;
vinfos[5].indices[0] = _ij7[0];
vinfos[5].indices[1] = _ij7[1];
vinfos[5].maxsolutions = _nj7;
std::vector<int> vfree(0);
solutions.AddSolution(vinfos,vfree);
}
}
}
}
}
}
}

}

}
}
}
}
}
}
}

}

}
}
return solutions.GetNumSolutions()>0;
}
};


/// solves the inverse kinematics equations.
/// \param pfree is an array specifying the free joints of the chain.
IKFAST_API bool ComputeIk(const IkReal* eetrans, const IkReal* eerot, const IkReal* pfree, IkSolutionListBase<IkReal>& solutions) {
IKSolver solver;
return solver.ComputeIk(eetrans,eerot,pfree,solutions);
}

IKFAST_API bool ComputeIk2(const IkReal* eetrans, const IkReal* eerot, const IkReal* pfree, IkSolutionListBase<IkReal>& solutions, void* pOpenRAVEManip) {
IKSolver solver;
return solver.ComputeIk(eetrans,eerot,pfree,solutions);
}

IKFAST_API const char* GetKinematicsHash() { return "<robot:GenericRobot - jaka_with_cam (65666f8df67ba7c3595de28fdf16526c)>"; }

IKFAST_API const char* GetIkFastVersion() { return "0x1000004b"; }

#ifdef IKFAST_NAMESPACE
} // end namespace
#endif

#ifndef IKFAST_NO_MAIN
#include <stdio.h>
#include <stdlib.h>
#ifdef IKFAST_NAMESPACE
using namespace IKFAST_NAMESPACE;
#endif
int main(int argc, char** argv)
{
    if( argc != 12+GetNumFreeParameters()+1 ) {
        printf("\nUsage: ./ik r00 r01 r02 t0 r10 r11 r12 t1 r20 r21 r22 t2 free0 ...\n\n"
               "Returns the ik solutions given the transformation of the end effector specified by\n"
               "a 3x3 rotation R (rXX), and a 3x1 translation (tX).\n"
               "There are %d free parameters that have to be specified.\n\n",GetNumFreeParameters());
        return 1;
    }

    IkSolutionList<IkReal> solutions;
    std::vector<IkReal> vfree(GetNumFreeParameters());
    IkReal eerot[9],eetrans[3];
    eerot[0] = atof(argv[1]); eerot[1] = atof(argv[2]); eerot[2] = atof(argv[3]); eetrans[0] = atof(argv[4]);
    eerot[3] = atof(argv[5]); eerot[4] = atof(argv[6]); eerot[5] = atof(argv[7]); eetrans[1] = atof(argv[8]);
    eerot[6] = atof(argv[9]); eerot[7] = atof(argv[10]); eerot[8] = atof(argv[11]); eetrans[2] = atof(argv[12]);
    for(std::size_t i = 0; i < vfree.size(); ++i)
        vfree[i] = atof(argv[13+i]);
    bool bSuccess = ComputeIk(eetrans, eerot, vfree.size() > 0 ? &vfree[0] : NULL, solutions);

    if( !bSuccess ) {
        fprintf(stderr,"Failed to get ik solution\n");
        return -1;
    }

    printf("Found %d ik solutions:\n", (int)solutions.GetNumSolutions());
    std::vector<IkReal> solvalues(GetNumJoints());
    for(std::size_t i = 0; i < solutions.GetNumSolutions(); ++i) {
        const IkSolutionBase<IkReal>& sol = solutions.GetSolution(i);
        printf("sol%d (free=%d): ", (int)i, (int)sol.GetFree().size());
        std::vector<IkReal> vsolfree(sol.GetFree().size());
        sol.GetSolution(&solvalues[0],vsolfree.size()>0?&vsolfree[0]:NULL);
        for( std::size_t j = 0; j < solvalues.size(); ++j)
            printf("%.15f, ", solvalues[j]);
        printf("\n");
    }
    return 0;
}

#endif
