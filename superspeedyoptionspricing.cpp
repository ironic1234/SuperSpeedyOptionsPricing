#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <thread>

using namespace std;

// Standard normal probability density function
double norm_pdf(const double& x) {
    return (1.0/(pow(2*M_PI,0.5)))*exp(-0.5*x*x);
}

// An approximation to the cumulative distribution function
// for the standard normal distribution
// Note: This is a recursive function
double norm_cdf(const double& x) {
    double k = 1.0/(1.0 + 0.2316419*x);
    double k_sum = k*(0.319381530 + k*(-0.356563782 + k*(1.781477937 + k*(-1.821255978 + 1.330274429*k))));

    if (x >= 0.0) {
        return (1.0 - (1.0/(pow(2*M_PI,0.5)))*exp(-0.5*x*x) * k_sum);
    } else {
        return 1.0 - norm_cdf(-x);
    }
}

// This calculates d_j, for j in {1,2}. This term appears in the closed
// form solution for the European call or put price
double d_j(const int& j, const double& S, const double& K, const double& r, const double& v, const double& T) {
    return (log(S/K) + (r + (pow(-1,j-1))*0.5*v*v)*T)/(v*(pow(T,0.5)));
}

// Calculate the European vanilla call price based on
// underlying S, strike K, risk-free rate r, volatility of
// underlying sigma and time to maturity T
double call_price(const double& S, const double& K, const double& r, const double& v, const double& T) {
    return S * norm_cdf(d_j(1, S, K, r, v, T))-K*exp(-r*T) * norm_cdf(d_j(2, S, K, r, v, T));
}

// Calculate the European vanilla call Delta
double call_delta(const double S, const double K, const double r, const double v, const double T) {
  return norm_cdf(d_j(1, S, K, r, v, T));
}

// Calculate the European vanilla call Gamma
double call_gamma(const double S, const double K, const double r, const double v, const double T) {
  return norm_pdf(d_j(1, S, K, r, v, T))/(S*v*sqrt(T));
}

// Calculate the European vanilla call Vega
double call_vega(const double S, const double K, const double r, const double v, const double T) {
  return S*norm_pdf(d_j(1, S, K, r, v, T))*sqrt(T);
}

// Calculate the European vanilla call Theta
double call_theta(const double S, const double K, const double r, const double v, const double T) {
  return -(S*norm_pdf(d_j(1, S, K, r, v, T))*v)/(2*sqrt(T)) 
    - r*K*exp(-r*T)*norm_cdf(d_j(2, S, K, r, v, T));
}

// Calculate the European vanilla call Rho
double call_rho(const double S, const double K, const double r, const double v, const double T) {
  return K*T*exp(-r*T)*norm_cdf(d_j(2, S, K, r, v, T));
}

// Calculate the European vanilla put price based on
// underlying S, strike K, risk-free rate r, volatility of
// underlying sigma and time to maturity T
double put_price(const double& S, const double& K, const double& r, const double& v, const double& T) {
    return -S*norm_cdf(-d_j(1, S, K, r, v, T))+K*exp(-r*T) * norm_cdf(-d_j(2, S, K, r, v, T));
}

// Calculate the European vanilla put Delta
double put_delta(const double S, const double K, const double r, const double v, const double T) {
  return norm_cdf(d_j(1, S, K, r, v, T)) - 1;
}

// Calculate the European vanilla put Gamma
double put_gamma(const double S, const double K, const double r, const double v, const double T) {
  return call_gamma(S, K, r, v, T); // Identical to call by put-call parity
}

// Calculate the European vanilla put Vega
double put_vega(const double S, const double K, const double r, const double v, const double T) {
  return call_vega(S, K, r, v, T); // Identical to call by put-call parity
}

// Calculate the European vanilla put Theta
double put_theta(const double S, const double K, const double r, const double v, const double T) {
  return -(S*norm_pdf(d_j(1, S, K, r, v, T))*v)/(2*sqrt(T)) 
    + r*K*exp(-r*T)*norm_cdf(-d_j(2, S, K, r, v, T));
}

// Calculate the European vanilla put Rho
double put_rho(const double S, const double K, const double r, const double v, const double T) {
  return -T*K*exp(-r*T)*norm_cdf(-d_j(2, S, K, r, v, T));
}

int createfile(const double K, const double R, const double T, const double Sbottom, const double Stop, const double Sincrement, const double Vbottom, const double Vtop, const double Vincrement) {
    ofstream file;
    file.open(to_string(K) + ".csv", ios::app);
    file << "Underlying,Strike,Riskfree,Volatility,Days to expiry,Price,Delta,Gamma,Vega,Theta,Rho\n";
    for (double S = Sbottom; S <= Stop; S += Sincrement) {
        for (double V = Vbottom; V <= Vtop; V += Vincrement) {
            for (double x = 1; x<=T; x++) {
                double call = call_price(S, K, R/100, V/100, x/365);
                double calldelta = call_delta(S, K, R/100, V/100, x/365);
                double callgamma = call_gamma(S, K, R/100, V/100, x/365);
                double callvega = call_vega(S, K, R/100, V/100, x/365);
                double calltheta = call_theta(S, K, R/100, V/100, x/365);
                double callrho = call_rho(S, K, R/100, V/100, x/365);
                file << S << "," << K << "," << R << "," << V << "," << x << "," << to_string(call) << "," << to_string(calldelta) << "," << to_string(callgamma) << "," << to_string(callvega) << "," << to_string(calltheta) << "," << to_string(callrho) << "\n";
                
            }
        }
    }
    return 0;
}

int main(int argc, char **argv) {

    // First we create the parameter list
    double Sbottom = 0;
    double Stop;  // Option price
    double Sincrement;
    double Kbottom = 0;
    double Ktop;  // Strike price
    double Kincrement;
    double R = 8;   // Risk-free rate (5%)
    double Vbottom = 0;
    double Vtop;   // Volatility of the underlying (20%)
    double Vincrement;
    double T;    // One year until expiry

    string filey = "";
    cout << "File?";
    cin >> filey;

    if (filey == "no") {
        cout << "Underlying: ";
        cin >> Sbottom;
        cout << "Strike: ";
        cin >> Kbottom;
        cout << "Volatility: ";
        cin >> Vbottom;
        cout << "Days to expiry: ";
        cin >> T;
        cout << "Price: " << call_price(Sbottom, Kbottom, R/100, Vbottom/100, T/365) << "\n";
        cout << "Delta: " << call_delta(Sbottom, Kbottom, R/100, Vbottom/100, T/365) << "\n";
        cout << "Gamma: " << call_gamma(Sbottom, Kbottom, R/100, Vbottom/100, T/365) << "\n";
        cout << "Vega: " << call_vega(Sbottom, Kbottom, R/100, Vbottom/100, T/365) << "\n";
        cout << "Theta: " << call_theta(Sbottom, Kbottom, R/100, Vbottom/100, T/365) << "\n";
        cout << "Rho: " << call_rho(Sbottom, Kbottom, R/100, Vbottom/100, T/365) << "\n";

        return 0;
    }

    cout << "Underlying start: ";
    cin >> Sbottom;
    cout << "Underlying end: ";
    cin >> Stop;
    cout << "Underlying increment: ";
    cin >> Sincrement;
    cout << "Strike start: ";
    cin >> Kbottom;
    cout << "Strike end: ";
    cin >> Ktop;
    cout << "Strike increment: ";
    cin >> Kincrement;
    cout << "Volatility start: ";
    cin >> Vbottom;
    cout << "Volatility end: ";
    cin >> Vtop;
    cout << "Volatility increment: ";
    cin >> Vincrement;
    cout << "Days to expiry: ";
    cin >> T;
    
    double S = Sbottom;
    double K = Kbottom;
    double V = Vbottom;

    auto start = chrono::high_resolution_clock::now();
    // Then we calculate the call/put values
    for (K = Kbottom; K <= Ktop; K += Kincrement) {
        createfile(K,R,T,Sbottom,Stop,Sincrement,Vbottom,Vtop,Vincrement);
    }
    auto stop = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

    cout << duration.count() << " microseconds\n";
    return 0;
}