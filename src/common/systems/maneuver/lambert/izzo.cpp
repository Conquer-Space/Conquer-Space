#include "common/systems/maneuver/lambert/izzo.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

#include <glm/glm.hpp>

#include "common/components/units.h"
#include "izzo.h"

cqsp::common::systems::lambert::Izzo::Izzo(const glm::dvec3& r1, const glm::dvec3& r2, double tof, double mu, int cw,
                                           int revs)
    : r1(r1), r2(r2), tof(tof), mu(mu), cw(cw), revs(revs) {}

// Code heavily derived from https://github.com/esa/pykep/blob/master/src/lambert_problem.cpp
glm::dvec3 cqsp::common::systems::lambert::Izzo::Solve(const glm::dvec3& v_start) {
    double c_mag = glm::length(r2 - r1);
    double r1_mag = glm::length(r1);
    double r2_mag = glm::length(r2);

    double s = 0.5 * (r1_mag + r2_mag + c_mag);

    glm::dvec3 i_r1 = r1 / r1_mag;
    glm::dvec3 i_r2 = r2 / r2_mag;
    glm::dvec3 i_h = glm::cross(i_r1, i_r2);
    i_h = glm::normalize(i_h);

    lambda2 = 1 - c_mag / s;
    lambda = std::sqrt(lambda2);
    glm::dvec3 i_t1;
    glm::dvec3 i_t2;
    if ((r1.x * r2.y - r1.y * r2.x) < 0.0) {  // Transfer angle is larger than 180 degrees as seen from above the z axis
        lambda = -lambda;
        i_t1 = glm::cross(i_r1, i_h);
        i_t2 = glm::cross(i_r2, i_h);
    } else {
        i_t1 = glm::cross(i_h, i_r1);
        i_t2 = glm::cross(i_h, i_r2);
    }
    i_t1 = glm::normalize(i_t1);
    i_t2 = glm::normalize(i_t2);

    if (cw) {
        // Retrograde motion
        lambda = -lambda;
        i_t1 = -i_t1;
        i_t2 = -i_t2;
    }
    lambda3 = lambda * lambda2;
    double T = sqrt(2 * mu / (s * s * s)) * tof;
    FindXY(lambda, T);

    // 4 - For each found x value we reconstruct the terminal velocities
    double gamma = sqrt(mu * s / 2.);
    double rho = (r1_mag - r2_mag) / c_mag;
    double sigma = sqrt(1 - rho * rho);
    for (size_t i = 0; i < x.size(); ++i) {
        double y = sqrt(1.0 - lambda2 + lambda2 * x[i] * x[i]);
        double vr1 = gamma * ((lambda * y - x[i]) - rho * (lambda * y + x[i])) / r1_mag;
        double vr2 = -gamma * ((lambda * y - x[i]) + rho * (lambda * y + x[i])) / r2_mag;
        double vt = gamma * sigma * (y + lambda * x[i]);
        double vt1 = vt / r1_mag;
        double vt2 = vt / r2_mag;
        v1[i] = vr1 * i_r1 + vt1 * i_t1;
        v2[i] = vr2 * i_r2 + vt2 * i_t2;
    }
    double min_dv = std::numeric_limits<double>::infinity();
    int lowest_v = -1;
    for (size_t i = 0; i < v1.size(); i++) {
        double t = glm::length(v1[i] - v_start);
        std::cout << "V1 " << v1[i].x << ", " << v1[i].y << ", " << v1[i].z << "\n";
        std::cout << "V2 " << v2[i].x << ", " << v2[i].y << ", " << v2[i].z << "\n";
        std::cout << x[i] << ", " << iters[i] << "\n";
        if (t < min_dv) {
            t = min_dv;
            lowest_v = i;
        }
    }
    // Then return lowest v
    if (lowest_v >= 0) {
        return v1[lowest_v];
    } else {
        return v1[0];
    }
}

void cqsp::common::systems::lambert::Izzo::FindXY(double lambda, double T) {
    // We now have lambda, T and we will find all x
    // Let's detect the number of revolutions for which there exists a solution
    int Nmax = static_cast<int>(T / components::types::PI);
    double T00 = acos(lambda) + lambda * sqrt(1. - lambda2);
    double T0 = (T00 + Nmax * components::types::PI);
    double T1 = 2. / 3. * (1. - lambda3);
    if (Nmax > 0) {
        if (T < T0) {
            int it = 0;
            double T_min = T0;
            double x_old = 0.0;
            double x_new = 0.0;
            while (true) {
                dTdx(DT, DDT, DDDT, x_old, T_min);
                if (DT != 0) {  // We use Halley iterations to find xM and TM
                    x_new = x_old - DT * DDT / (DDT * DDT - DT * DDDT / 2.0);
                }
                double err = fabs(x_old - x_new);
                if ((err < 1e-13) || (it > 12)) {
                    break;
                }
                T_min = x2tof(x_new, Nmax);
                x_old = x_new;
                it++;
            }
            if (T_min > T) {
                Nmax--;
            }
        }
    }
    // We exit this if clause with Nmax being the maximum number of revolutions
    // for which there exists a solution. We crop it to revs
    Nmax = (int)std::min((revs), Nmax);

    v1.resize(static_cast<size_t>(Nmax) * 2 + 1);
    v2.resize(static_cast<size_t>(Nmax) * 2 + 1);
    iters.resize(static_cast<size_t>(Nmax) * 2 + 1);
    x.resize(static_cast<size_t>(Nmax) * 2 + 1);

    // Find all solutions in x, y
    // 0 rev solution
    // Get initial guess
    if (T >= T00) {
        x[0] = -(T - T00) / (T - T00 + 4);
    } else if (T <= T1) {
        x[0] = T1 * (T1 - T) / (2. / 5. * (1 - lambda2 * lambda3) * T) + 1;
    } else {
        x[0] = pow(T / T00, 0.69314718055994529 / log(T1 / T00)) - 1;  // 0.69314718055994529 = ln 2
    }

    // Householder iterations
    iters[0] = householder(T, x[0], 0, 1e-5, 15);

    // Multi rev solutions
    for (decltype(Nmax) i = 1; i < Nmax + 1; i++) {
        // Left householder iterations
        double tmp = pow((i * components::types::PI + components::types::PI) / (8. * T), 2. / 3.);
        x[2 * i - 1] = (tmp - 1) / (tmp + 1);
        iters[2 * i - 1] = householder(T, x[2 * i - 1], i, 1e-8, 15);
        // Right householder iterations
        tmp = pow((8. * T) / (i * components::types::PI), 2. / 3.);
        x[2 * i] = (tmp - 1) / (tmp + 1);
        iters[2 * i] = householder(T, x[2 * i], i, 1e-8, 15);
    }
}

void cqsp::common::systems::lambert::Izzo::dTdx(double& DT, double& DDT, double DDDT, const double x, const double T) {
    double l2 = lambda * lambda;
    double l3 = l2 * lambda;
    double umx2 = 1.0 - x * x;
    double y = sqrt(1.0 - l2 * umx2);
    double y2 = y * y;
    double y3 = y * y2;
    DT = 1. / umx2 * (3. * T * x - 2.0 + 2.0 * l3 * x / y);
    DDT = 1.0 / umx2 * (3. * T + 5. * x * DT + 2. * (1. - l2) * l3 / y3);
    DDDT = 1. / umx2 * (7. * x * DDT + 8. * DT - 6. * (1. * l2) * l2 * l3 * x / y3 / y2);
}

int cqsp::common::systems::lambert::Izzo::householder(double& x0, const double T, const int N, const double eps,
                                                      const int iter_max) {
    int it = 0;
    double err = 1.0;
    double xnew = 0.0;
    double tof = 0.0, delta = 0.0, DT = 0.0, DDT = 0.0, DDDT = 0.0;
    while ((err > eps) && (it < iter_max)) {
        tof = x2tof(x0, N);
        dTdx(DT, DDT, DDDT, x0, tof);
        delta = tof - T;
        double DT2 = DT * DT;
        xnew = x0 - delta * (DT2 - delta * DDT / 2.0) / (DT * (DT2 - delta * DDT) + DDDT * delta * delta / 6.0);
        err = fabs(x0 - xnew);
        x0 = xnew;
        it++;
    }
    return it;
}

double cqsp::common::systems::lambert::Izzo::x2tof2(const double x, const int N) {
    double a = 1.0 / (1.0 - x * x);
    if (a > 0) {  // ellipse
        double alfa = 2.0 * acos(x);
        double beta = 2.0 * asin(sqrt(lambda * lambda / a));
        if (lambda < 0.0) beta = -beta;
        return ((a * sqrt(a) * ((alfa - sin(alfa)) - (beta - sin(beta)) + 2.0 * components::types::PI * N)) / 2.0);
    } else {
        double alfa = 2.0 * acosh(x);
        double beta = 2.0 * asinh(sqrt(-lambda * lambda / a));
        if (lambda < 0.0) beta = -beta;
        return (-a * sqrt(-a) * ((beta - sinh(beta)) - (alfa - sinh(alfa))) / 2.0);
    }
}

double cqsp::common::systems::lambert::Izzo::x2tof(const double x, const int N) {
    const double battin = 0.01;
    const double lagrange = 0.2;
    double dist = fabs(x - 1);
    if (dist < lagrange && dist > battin) {  // We use Lagrange tof expression
        return x2tof2(x, N);
    }
    double K = lambda * lambda;
    double E = x * x - 1.0;
    double rho = fabs(E);
    double z = sqrt(1 + K * E);
    if (dist < battin) {  // We use Battin series tof expression
        double eta = z - lambda * x;
        double S1 = 0.5 * (1.0 - lambda - x * eta);
        double Q = hypergeometricF(S1, 1e-11);
        Q = 4.0 / 3.0 * Q;
        return (eta * eta * eta * Q + 4.0 * lambda * eta) / 2.0 + N * components::types::PI / pow(rho, 1.5);
    } else {  // We use Lancaster tof expresion
        double y = sqrt(rho);
        double g = x * z - lambda * E;
        double d = 0.0;
        if (E < 0) {
            double l = acos(g);
            d = N * components::types::PI + l;
        } else {
            double f = y * (z - lambda * x);
            d = log(f + g);
        }
        return (x - lambda * z - d / y) / E;
    }
}

double cqsp::common::systems::lambert::Izzo::hypergeometricF(double z, double tol) {
    double Sj = 1.0;
    double Cj = 1.0;
    double err = 1.0;
    double Cj1 = 0.0;
    double Sj1 = 0.0;
    int j = 0;
    while (err > tol) {
        Cj1 = Cj * (3.0 + j) * (1.0 + j) / (2.5 + j) * z / (j + 1);
        Sj1 = Sj + Cj1;
        err = fabs(Cj1);
        Sj = Sj1;
        Cj = Cj1;
        j++;
    }
    return Sj;
}
