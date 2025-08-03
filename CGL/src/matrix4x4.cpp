#include "matrix4x4.h"

#include <cmath>
#include <iostream>


using namespace std;

namespace CGL {

double &Matrix4x4::operator()(int i, int j) { return entries[j][i]; }

const double &Matrix4x4::operator()(int i, int j) const {
  return entries[j][i];
}

Vector4D &Matrix4x4::operator[](int j) { return entries[j]; }

const Vector4D &Matrix4x4::operator[](int j) const { return entries[j]; }

void Matrix4x4::zero(double val) {
  // sets all elements to val
  entries[0] = Vector4D(val);
  entries[1] = Vector4D(val);
  entries[2] = Vector4D(val);
  entries[3] = Vector4D(val);
}

double Matrix4x4::det(void) const {
  const Matrix4x4 &A(*this);

  return A(0, 3) * A(1, 2) * A(2, 1) * A(3, 0) -
         A(0, 2) * A(1, 3) * A(2, 1) * A(3, 0) -
         A(0, 3) * A(1, 1) * A(2, 2) * A(3, 0) +
         A(0, 1) * A(1, 3) * A(2, 2) * A(3, 0) +
         A(0, 2) * A(1, 1) * A(2, 3) * A(3, 0) -
         A(0, 1) * A(1, 2) * A(2, 3) * A(3, 0) -
         A(0, 3) * A(1, 2) * A(2, 0) * A(3, 1) +
         A(0, 2) * A(1, 3) * A(2, 0) * A(3, 1) +
         A(0, 3) * A(1, 0) * A(2, 2) * A(3, 1) -
         A(0, 0) * A(1, 3) * A(2, 2) * A(3, 1) -
         A(0, 2) * A(1, 0) * A(2, 3) * A(3, 1) +
         A(0, 0) * A(1, 2) * A(2, 3) * A(3, 1) +
         A(0, 3) * A(1, 1) * A(2, 0) * A(3, 2) -
         A(0, 1) * A(1, 3) * A(2, 0) * A(3, 2) -
         A(0, 3) * A(1, 0) * A(2, 1) * A(3, 2) +
         A(0, 0) * A(1, 3) * A(2, 1) * A(3, 2) +
         A(0, 1) * A(1, 0) * A(2, 3) * A(3, 2) -
         A(0, 0) * A(1, 1) * A(2, 3) * A(3, 2) -
         A(0, 2) * A(1, 1) * A(2, 0) * A(3, 3) +
         A(0, 1) * A(1, 2) * A(2, 0) * A(3, 3) +
         A(0, 2) * A(1, 0) * A(2, 1) * A(3, 3) -
         A(0, 0) * A(1, 2) * A(2, 1) * A(3, 3) -
         A(0, 1) * A(1, 0) * A(2, 2) * A(3, 3) +
         A(0, 0) * A(1, 1) * A(2, 2) * A(3, 3);
}

double Matrix4x4::norm(void) const {
  return sqrt(entries[0].norm2() + entries[1].norm2() + entries[2].norm2() +
              entries[3].norm2());
}

Matrix4x4 Matrix4x4::operator-(void) const {
  // returns -A (Negation).
  const Matrix4x4 &A(*this);
  Matrix4x4 B;

  B[0] = -A[0];
  B[1] = -A[1];
  B[2] = -A[2];
  B[3] = -A[3];

  return B;
}

void Matrix4x4::operator+=(const Matrix4x4 &B) {

  Matrix4x4 &A(*this);
  double *Aij = (double *)&A;
  const double *Bij = (const double *)&B;

  A[0] += B[0];
  A[1] += B[1];
  A[2] += B[2];
  A[3] += B[3];
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4 &B) const {
  const Matrix4x4 &A(*this);
  Matrix4x4 C;

  C[0] = A[0] - B[0];
  C[1] = A[1] - B[1];
  C[2] = A[2] - B[2];
  C[3] = A[3] - B[3];

  return C;
}

Matrix4x4 Matrix4x4::operator*(double c) const {
  const Matrix4x4 &A(*this);
  Matrix4x4 B;

  B[0] = c * A[0];
  B[1] = c * A[1];
  B[2] = c * A[2];
  B[3] = c * A[3];

  return B;
}

// Returns c*A.
Matrix4x4 operator*(double c, const Matrix4x4 &A) {
  Matrix4x4 cA;

  cA[0] = c * A[0];
  cA[1] = c * A[1];
  cA[2] = c * A[2];
  cA[3] = c * A[3];

  return cA;
}

// Tradiational Grade School Multiplication. N^3
Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &B) const {
  const Matrix4x4 &A(*this);
  Matrix4x4 C;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
#ifdef __AVX__
      C(i, j) = dot(Vector4D(A(i, 0), A(i, 1), A(i, 2), A(i, 3)), B[j]);
#else
      C(i, j) = 0.;

      for (int k = 0; k < 4; k++) {
        C(i, j) += A(i, k) * B(k, j);
      }
#endif
    }
  }

  return C;
}

Vector4D Matrix4x4::operator*(const Vector4D &x) const {
  return x[0] * entries[0] + // Add up products for each matrix column.
         x[1] * entries[1] + x[2] * entries[2] + x[3] * entries[3];
}

// Naive Transposition.
Matrix4x4 Matrix4x4::T(void) const {
  const Matrix4x4 &A(*this);
  Matrix4x4 B;

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      B(i, j) = A(j, i);
    }

  return B;
}

Matrix4x4 Matrix4x4::inv(void) const {
  const Matrix4x4 &A(*this);
  Matrix4x4 B;

  // Hardcoded in Fully Symbolic computation.

  B(0, 0) = A(1, 2) * A(2, 3) * A(3, 1) - A(1, 3) * A(2, 2) * A(3, 1) +
            A(1, 3) * A(2, 1) * A(3, 2) - A(1, 1) * A(2, 3) * A(3, 2) -
            A(1, 2) * A(2, 1) * A(3, 3) + A(1, 1) * A(2, 2) * A(3, 3);
  B(0, 1) = A(0, 3) * A(2, 2) * A(3, 1) - A(0, 2) * A(2, 3) * A(3, 1) -
            A(0, 3) * A(2, 1) * A(3, 2) + A(0, 1) * A(2, 3) * A(3, 2) +
            A(0, 2) * A(2, 1) * A(3, 3) - A(0, 1) * A(2, 2) * A(3, 3);
  B(0, 2) = A(0, 2) * A(1, 3) * A(3, 1) - A(0, 3) * A(1, 2) * A(3, 1) +
            A(0, 3) * A(1, 1) * A(3, 2) - A(0, 1) * A(1, 3) * A(3, 2) -
            A(0, 2) * A(1, 1) * A(3, 3) + A(0, 1) * A(1, 2) * A(3, 3);
  B(0, 3) = A(0, 3) * A(1, 2) * A(2, 1) - A(0, 2) * A(1, 3) * A(2, 1) -
            A(0, 3) * A(1, 1) * A(2, 2) + A(0, 1) * A(1, 3) * A(2, 2) +
            A(0, 2) * A(1, 1) * A(2, 3) - A(0, 1) * A(1, 2) * A(2, 3);
  B(1, 0) = A(1, 3) * A(2, 2) * A(3, 0) - A(1, 2) * A(2, 3) * A(3, 0) -
            A(1, 3) * A(2, 0) * A(3, 2) + A(1, 0) * A(2, 3) * A(3, 2) +
            A(1, 2) * A(2, 0) * A(3, 3) - A(1, 0) * A(2, 2) * A(3, 3);
  B(1, 1) = A(0, 2) * A(2, 3) * A(3, 0) - A(0, 3) * A(2, 2) * A(3, 0) +
            A(0, 3) * A(2, 0) * A(3, 2) - A(0, 0) * A(2, 3) * A(3, 2) -
            A(0, 2) * A(2, 0) * A(3, 3) + A(0, 0) * A(2, 2) * A(3, 3);
  B(1, 2) = A(0, 3) * A(1, 2) * A(3, 0) - A(0, 2) * A(1, 3) * A(3, 0) -
            A(0, 3) * A(1, 0) * A(3, 2) + A(0, 0) * A(1, 3) * A(3, 2) +
            A(0, 2) * A(1, 0) * A(3, 3) - A(0, 0) * A(1, 2) * A(3, 3);
  B(1, 3) = A(0, 2) * A(1, 3) * A(2, 0) - A(0, 3) * A(1, 2) * A(2, 0) +
            A(0, 3) * A(1, 0) * A(2, 2) - A(0, 0) * A(1, 3) * A(2, 2) -
            A(0, 2) * A(1, 0) * A(2, 3) + A(0, 0) * A(1, 2) * A(2, 3);
  B(2, 0) = A(1, 1) * A(2, 3) * A(3, 0) - A(1, 3) * A(2, 1) * A(3, 0) +
            A(1, 3) * A(2, 0) * A(3, 1) - A(1, 0) * A(2, 3) * A(3, 1) -
            A(1, 1) * A(2, 0) * A(3, 3) + A(1, 0) * A(2, 1) * A(3, 3);
  B(2, 1) = A(0, 3) * A(2, 1) * A(3, 0) - A(0, 1) * A(2, 3) * A(3, 0) -
            A(0, 3) * A(2, 0) * A(3, 1) + A(0, 0) * A(2, 3) * A(3, 1) +
            A(0, 1) * A(2, 0) * A(3, 3) - A(0, 0) * A(2, 1) * A(3, 3);
  B(2, 2) = A(0, 1) * A(1, 3) * A(3, 0) - A(0, 3) * A(1, 1) * A(3, 0) +
            A(0, 3) * A(1, 0) * A(3, 1) - A(0, 0) * A(1, 3) * A(3, 1) -
            A(0, 1) * A(1, 0) * A(3, 3) + A(0, 0) * A(1, 1) * A(3, 3);
  B(2, 3) = A(0, 3) * A(1, 1) * A(2, 0) - A(0, 1) * A(1, 3) * A(2, 0) -
            A(0, 3) * A(1, 0) * A(2, 1) + A(0, 0) * A(1, 3) * A(2, 1) +
            A(0, 1) * A(1, 0) * A(2, 3) - A(0, 0) * A(1, 1) * A(2, 3);
  B(3, 0) = A(1, 2) * A(2, 1) * A(3, 0) - A(1, 1) * A(2, 2) * A(3, 0) -
            A(1, 2) * A(2, 0) * A(3, 1) + A(1, 0) * A(2, 2) * A(3, 1) +
            A(1, 1) * A(2, 0) * A(3, 2) - A(1, 0) * A(2, 1) * A(3, 2);
  B(3, 1) = A(0, 1) * A(2, 2) * A(3, 0) - A(0, 2) * A(2, 1) * A(3, 0) +
            A(0, 2) * A(2, 0) * A(3, 1) - A(0, 0) * A(2, 2) * A(3, 1) -
            A(0, 1) * A(2, 0) * A(3, 2) + A(0, 0) * A(2, 1) * A(3, 2);
  B(3, 2) = A(0, 2) * A(1, 1) * A(3, 0) - A(0, 1) * A(1, 2) * A(3, 0) -
            A(0, 2) * A(1, 0) * A(3, 1) + A(0, 0) * A(1, 2) * A(3, 1) +
            A(0, 1) * A(1, 0) * A(3, 2) - A(0, 0) * A(1, 1) * A(3, 2);
  B(3, 3) = A(0, 1) * A(1, 2) * A(2, 0) - A(0, 2) * A(1, 1) * A(2, 0) +
            A(0, 2) * A(1, 0) * A(2, 1) - A(0, 0) * A(1, 2) * A(2, 1) -
            A(0, 1) * A(1, 0) * A(2, 2) + A(0, 0) * A(1, 1) * A(2, 2);

  // Invertable iff the determinant is not equal to zero.
  B /= det();

  return B;
}

void Matrix4x4::operator/=(double x) {
  Matrix4x4 &A(*this);
  double rx = 1. / x;

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      A(i, j) *= rx;
    }
}


Matrix4x4 Matrix4x4::perspective(double fovy, double aspect, double near, double far) {
    Matrix4x4 m;
    m.zero(0.0);

    double theta = fovy / 2.0;
    double cot_theta = 1.0 / tan(theta);

    m(0, 0) = cot_theta / aspect;
    m(1, 1) = cot_theta;
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = -2 * far * near / (far - near);
    m(3, 2) = -1.0;

    return m;
}

Matrix4x4 Matrix4x4::lookAt(const Vector3D &eye, const Vector3D &at, const Vector3D &up) {
    Vector3D f = (at - eye).unit();
    Vector3D s = cross(f, up).unit();
    Vector3D u = cross(s, f);

    Matrix4x4 m;
    m.column(0) = Vector4D(s.x, u.x, -f.x, 0);
    m.column(1) = Vector4D(s.y, u.y, -f.y, 0);
    m.column(2) = Vector4D(s.z, u.z, -f.z, 0);
    m.column(3) = Vector4D(-dot(s, eye), -dot(u, eye), dot(f, eye), 1.0);

    return m;
}

Matrix4x4 Matrix4x4::scale(double x, double y, double z) {
    Matrix4x4 m;
    m.entries[0] = Vector4D(x, 0, 0, 0);
    m.entries[1] = Vector4D(0, y, 0, 0);
    m.entries[2] = Vector4D(0, 0, z, 0);
    m.entries[3] = Vector4D(0, 0, 0, 1);
    return m;
}

Matrix4x4 Matrix4x4::rotateX(double r) {
    Matrix4x4 m;
    double c = cos(r);
    double s = sin(r);
    m.entries[0] = Vector4D(1, 0, 0, 0);
    m.entries[1] = Vector4D(0, c, s, 0);
    m.entries[2] = Vector4D(0, -s, c, 0);
    m.entries[3] = Vector4D(0, 0, 0, 1);
    return m;
}
Matrix4x4 Matrix4x4::rotateY(double r) {
    Matrix4x4 m;
    double c = cos(r);
    double s = sin(r);
    m.entries[0] = Vector4D(c, 0, s, 0);
    m.entries[1] = Vector4D(0, 1, 0, 0);
    m.entries[2] = Vector4D(-s, 0, c, 0);
    m.entries[3] = Vector4D(0, 0, 0, 1);
    return m;
}
Matrix4x4 Matrix4x4::rotateZ(double r) {
    Matrix4x4 m;
    double c = cos(r);
    double s = sin(r);
    m.entries[0] = Vector4D(c, s, 0, 0);
    m.entries[1] = Vector4D(-s, c, 0, 0);
    m.entries[2] = Vector4D(0, 0, 1, 0);
    m.entries[3] = Vector4D(0, 0, 0, 1);
    return m;
}
Matrix4x4 Matrix4x4::translation(double x, double y, double z) {
    Matrix4x4 m;
    m.entries[0] = Vector4D(1, 0, 0, 0);
    m.entries[1] = Vector4D(0, 1, 0, 0);
    m.entries[2] = Vector4D(0, 0, 1, 0);
    m.entries[3] = Vector4D(x, y, z, 1);
    return m;
}

Matrix4x4 Matrix4x4::identity(void) {
  Matrix4x4 B;

  B(0, 0) = 1.;
  B(0, 1) = 0.;
  B(0, 2) = 0.;
  B(0, 3) = 0.;
  B(1, 0) = 0.;
  B(1, 1) = 1.;
  B(1, 2) = 0.;
  B(1, 3) = 0.;
  B(2, 0) = 0.;
  B(2, 1) = 0.;
  B(2, 2) = 1.;
  B(2, 3) = 0.;
  B(3, 0) = 0.;
  B(3, 1) = 0.;
  B(3, 2) = 0.;
  B(3, 3) = 1.;

  return B;
}

Matrix4x4 outer(const Vector4D &u, const Vector4D &v) {
  Matrix4x4 B;

  // Opposite of an inner product.
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      B(i, j) = u[i] * v[j];
    }

  return B;
}

std::ostream &operator<<(std::ostream &os, const Matrix4x4 &A) {
  for (int i = 0; i < 4; i++) {
    os << "[ ";

    for (int j = 0; j < 4; j++) {
      os << A(i, j) << " ";
    }

    os << "]" << std::endl;
  }

  return os;
}

Vector4D &Matrix4x4::column(int i) { return entries[i]; }

const Vector4D &Matrix4x4::column(int i) const { return entries[i]; }
} // namespace CGL
