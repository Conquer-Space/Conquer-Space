/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "common/actions/maneuver/lambert/izzo.h"

#include <cstddef>
#include <numbers>
#include <ostream>

namespace cqsp::common::systems::lambert {
/** Constructs and solves a Lambert problem.
  *
  * \param[in] R1 first cartesian position
  * \param[in] R2 second cartesian position
  * \param[in] tof time of flight
  * \param[in] mu gravity parameter
  * \param[in] cw when 1 a retrograde orbit is assumed
  * \param[in] multi_revs maximum number of multirevolutions to compute
  */
Izzo::Izzo(const glm::dvec3 &r1, const glm::dvec3 &r2, const double &tof, const double &mu, const bool cw,
           const int &multi_revs)
    : r1(r1), r2(r2), tof(tof), mu(mu), m_has_converged(true), m_multi_revs(multi_revs), cw(cw) {}

void Izzo::solve() {
    // 0 - Sanity checks
    if (tof <= 0) {
        //  throw_value_error("Time of flight is negative!");
    }
    if (mu <= 0) {
        //  throw_value_error("Gravity parameter is zero or negative!");
    }
    // 1 - Getting lambda and T
    m_c =
        sqrt((r2[0] - r1[0]) * (r2[0] - r1[0]) + (r2[1] - r1[1]) * (r2[1] - r1[1]) + (r2[2] - r1[2]) * (r2[2] - r1[2]));
    double R1 = glm::length(r1);
    double R2 = glm::length(r2);
    m_s = (m_c + R1 + R2) / 2.0;
    glm::dvec3 ir1;
    glm::dvec3 ir2;
    glm::dvec3 ih;
    glm::dvec3 it1;
    glm::dvec3 it2;
    ir1 = r1 / R1;
    ir2 = r2 / R2;
    ih = glm::cross(ir1, ir2);

    ih = glm::normalize(ih);
    if (ih[2] == 0) {
        //  throw_value_error("The angular momentum vector has no z component, impossible to define automatically clock or "
        //                    "counterclockwise");
    }
    double lambda2 = 1.0 - m_c / m_s;
    m_lambda = sqrt(lambda2);

    if (ih[2] < 0.0)  // Transfer angle is larger than 180 degrees as seen from abive the z axis
    {
        m_lambda = -m_lambda;
        it1 = glm::cross(ir1, ih);
        it2 = glm::cross(ir2, ih);
    } else {
        it1 = glm::cross(ih, ir1);
        it2 = glm::cross(ih, ir2);
    }
    it1 = glm::normalize(it1);
    it2 = glm::normalize(it2);

    if (cw) {  // Retrograde motion
        m_lambda = -m_lambda;
        it1 = -it1;
        it2 = -it2;
    }
    double lambda3 = m_lambda * lambda2;
    double T = sqrt(2.0 * mu / m_s / m_s / m_s) * tof;

    // 2 - We now have lambda, T and we will find all x
    // 2.1 - Let us first detect the maximum number of revolutions for which there exists a solution
    m_Nmax = static_cast<int>(T / std::numbers::pi);
    double T00 = acos(m_lambda) + m_lambda * sqrt(1.0 - lambda2);
    double T0 = (T00 + m_Nmax * std::numbers::pi);
    double T1 = 2.0 / 3.0 * (1.0 - lambda3);
    double DT = 0.0;
    double DDT = 0.0;
    double DDDT = 0.0;
    if (m_Nmax > 0) {
        if (T < T0) {  // We use Halley iterations to find xM and TM
            int it = 0;
            double err = 1.0;
            double T_min = T0;
            double x_old = 0.0;
            double x_new = 0.0;
            while (true) {
                dTdx(DT, DDT, DDDT, x_old, T_min);
                if (DT != 0.0) {
                    x_new = x_old - DT * DDT / (DDT * DDT - DT * DDDT / 2.0);
                }
                err = fabs(x_old - x_new);
                if ((err < 1e-13) || (it > 12)) {
                    break;
                }
                x2tof(T_min, x_new, m_Nmax);
                x_old = x_new;
                it++;
            }
            if (T_min > T) {
                m_Nmax -= 1;
            }
        }
    }
    // We exit this if clause with Nmax being the maximum number of revolutions
    // for which there exists a solution. We crop it to m_multi_revs
    m_Nmax = std::min(m_multi_revs, m_Nmax);

    // 2.2 We now allocate the memory for the output variables
    m_v1.resize(static_cast<size_t>(m_Nmax) * 2 + 1);
    m_v2.resize(static_cast<size_t>(m_Nmax) * 2 + 1);
    m_iters.resize(static_cast<size_t>(m_Nmax) * 2 + 1);
    m_x.resize(static_cast<size_t>(m_Nmax) * 2 + 1);

    // 3 - We may now find all solutions in x,y
    // 3.1 0 rev solution
    // 3.1.1 initial guess
    if (T >= T00) {
        m_x[0] = -(T - T00) / (T - T00 + 4);
    } else if (T <= T1) {
        m_x[0] = T1 * (T1 - T) / (2.0 / 5.0 * (1 - lambda2 * lambda3) * T) + 1;
    } else {
        m_x[0] = pow((T / T00), std::numbers::ln2 / log(T1 / T00)) - 1.0;
    }
    // 3.1.2 Householder iterations
    m_iters[0] = householder(T, m_x[0], 0, 1e-5, 15);
    // 3.2 multi rev solutions
    double tmp;
    for (size_t i = 1; i < m_Nmax + 1; ++i) {
        // 3.2.1 left Householder iterations
        tmp = pow((i * std::numbers::pi + std::numbers::pi) / (8.0 * T), 2.0 / 3.0);
        m_x[2 * i - 1] = (tmp - 1) / (tmp + 1);
        m_iters[2 * i - 1] = householder(T, m_x[2 * i - 1], i, 1e-8, 15);
        // 3.2.1 right Householder iterations
        tmp = pow((8.0 * T) / (i * std::numbers::pi), 2.0 / 3.0);
        m_x[2 * i] = (tmp - 1) / (tmp + 1);
        m_iters[2 * i] = householder(T, m_x[2 * i], i, 1e-8, 15);
    }

    // 4 - For each found x value we reconstruct the terminal velocities
    double gamma = sqrt(mu * m_s / 2.0);
    double rho = (R1 - R2) / m_c;
    double sigma = sqrt(1 - rho * rho);
    double vr1;
    double vt1;
    double vr2;
    double vt2;
    double y;
    for (size_t i = 0; i < m_x.size(); ++i) {
        y = sqrt(1.0 - lambda2 + lambda2 * m_x[i] * m_x[i]);
        vr1 = gamma * ((m_lambda * y - m_x[i]) - rho * (m_lambda * y + m_x[i])) / R1;
        vr2 = -gamma * ((m_lambda * y - m_x[i]) + rho * (m_lambda * y + m_x[i])) / R2;
        double vt = gamma * sigma * (y + m_lambda * m_x[i]);
        vt1 = vt / R1;
        vt2 = vt / R2;
        for (int j = 0; j < 3; ++j) m_v1[i][j] = vr1 * ir1[j] + vt1 * it1[j];
        for (int j = 0; j < 3; ++j) m_v2[i][j] = vr2 * ir2[j] + vt2 * it2[j];
    }
}

int Izzo::householder(const double T, double &x0, const int N, const double eps, const int iter_max) {
    int it = 0;
    double err = 1.0;
    double xnew = 0.0;
    double tof = 0.0;
    double delta = 0.0;
    double DT = 0.0;
    double DDT = 0.0;
    double DDDT = 0.0;
    while ((err > eps) && (it < iter_max)) {
        x2tof(tof, x0, N);
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

void Izzo::dTdx(double &DT, double &DDT, double &DDDT, const double x, const double T) {
    double l2 = m_lambda * m_lambda;
    double l3 = l2 * m_lambda;
    double umx2 = 1.0 - x * x;
    double y = sqrt(1.0 - l2 * umx2);
    double y2 = y * y;
    double y3 = y2 * y;
    DT = 1.0 / umx2 * (3.0 * T * x - 2.0 + 2.0 * l3 * x / y);
    DDT = 1.0 / umx2 * (3.0 * T + 5.0 * x * DT + 2.0 * (1.0 - l2) * l3 / y3);
    DDDT = 1.0 / umx2 * (7.0 * x * DDT + 8.0 * DT - 6.0 * (1.0 - l2) * l2 * l3 * x / y3 / y2);
}

void Izzo::x2tof2(double &tof, const double x, const int N) {
    double a = 1.0 / (1.0 - x * x);
    if (a > 0)  // ellipse
    {
        double alfa = 2.0 * acos(x);
        double beta = 2.0 * asin(sqrt(m_lambda * m_lambda / a));
        if (m_lambda < 0.0) beta = -beta;
        tof = ((a * sqrt(a) * ((alfa - sin(alfa)) - (beta - sin(beta)) + 2.0 * std::numbers::pi * N)) / 2.0);
    } else {
        double alfa = 2.0 * acosh(x);
        double beta = 2.0 * asinh(sqrt(-m_lambda * m_lambda / a));
        if (m_lambda < 0.0) beta = -beta;
        tof = (-a * sqrt(-a) * ((beta - sinh(beta)) - (alfa - sinh(alfa))) / 2.0);
    }
}

void Izzo::x2tof(double &tof, const double x, const int N) {
    double battin = 0.01;
    double lagrange = 0.2;
    double dist = fabs(x - 1);
    if (dist < lagrange && dist > battin) {  // We use Lagrange tof expression
        x2tof2(tof, x, N);
        return;
    }
    double K = m_lambda * m_lambda;
    double E = x * x - 1.0;
    double rho = fabs(E);
    double z = sqrt(1 + K * E);
    if (dist < battin) {  // We use Battin series tof expression
        double eta = z - m_lambda * x;
        double S1 = 0.5 * (1.0 - m_lambda - x * eta);
        double Q = hypergeometricF(S1, 1e-11);
        Q = 4.0 / 3.0 * Q;
        tof = (eta * eta * eta * Q + 4.0 * m_lambda * eta) / 2.0 + N * std::numbers::pi / pow(rho, 1.5);
        return;
    } else {  // We use Lancaster tof expresion
        double y = sqrt(rho);
        double g = x * z - m_lambda * E;
        double d = 0.0;
        if (E < 0) {
            double l = acos(g);
            d = N * std::numbers::pi + l;
        } else {
            double f = y * (z - m_lambda * x);
            d = log(f + g);
        }
        tof = (x - m_lambda * z - d / y) / E;
        return;
    }
}

double Izzo::hypergeometricF(double z, double tol) {
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
        j = j + 1;
    }
    return Sj;
}

/**
  *
  * \return an std::vector containing 3-d arrays with the cartesian components of the velocities at r1 for all 2N_max+1
  * solutions
  */
const std::vector<glm::dvec3> &Izzo::get_v1() const { return m_v1; }

/**
  *
  * \return an std::vector containing 3-d arrays with the cartesian components of the velocities at r2 for all 2N_max+1
  * solutions
  */
const std::vector<glm::dvec3> &Izzo::get_v2() const { return m_v2; }

/**
  *
  * \return a 3-d array with the cartesian components of r1
  */
const glm::dvec3 &Izzo::get_r1() const { return r1; }

/**
  *
  * \return a 3-d array with the cartesian components of r2
  */
const glm::dvec3 &Izzo::get_r2() const { return r2; }

/**
  *
  * \return the time of flight
  */
const double &Izzo::get_tof() const { return tof; }

/**
  * Gets the x variable for each solution found (0 revs, 1,1,2,2,3,3 .... N,N)
  *
  * \return the x variables in an std::vector
  */
const std::vector<double> &Izzo::get_x() const { return m_x; }

/**
  *
  * \return the gravitational parameter
  */
const double &Izzo::get_mu() const { return mu; }

/**
  *
  * \return an std::vector containing the iterations taken to compute each one of the solutions
  */
const std::vector<int> &Izzo::get_iters() const { return m_iters; }

/**
  *
  * \return the maximum number of revolutions. The number of solutions to the problem will be Nmax*2 +1
  */
int Izzo::get_Nmax() const { return m_Nmax; }
}  // namespace cqsp::common::systems::lambert
