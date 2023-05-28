#pragma once

#include "ustd_platform.h"
#include "ustd_array.h"

namespace ustd {
#define SENSOR_VALUE_INVALID -999999.0

/*!  \brief muwerk sensorprocessor class

sensorprocessor implements an exponential filter that smoothens and
    * throttles raw sensor data.
    *
    * It can be configured by:
    * * size of smoothing interval: larger intervals generate larger averaging
    * and slower response to change
    * * minimum change eps required to generate a new sensor reading, in order
    * to ignore small fluctuations
    * * a time-interval in seconds that generates a new sensor reading
    * regardless of change.
    *
    * This library requires the ustd library (for timeDiff) and requires a
    * <a href="https://github.com/muwerk/ustd/blob/master/README.md">platform
    * define</a>.
    *
    * Example:

~~~{.cpp}
void setup() {
    // generate a filter that exponentially averages over 10 values,
    // generates a new reading at least every 3600sec (even on no
    // change)
    // and generates a new reading every time, the filter value changes
    // for more than 0.1
    ustd::sensorprocessor mySensor(10,3600,0.1)
}

void loop() {
    double rawValue=ReadMyRawSensor();
    double filtered=rawValue;
    if mySensor.filter(&filtered) {
        printf("We got a new, filtered reading: %f\n", filtered);
    } else {
        // no valid new reading, do nothing.
    }
}
~~~
*/

class sensorprocessor {
  public:
    unsigned int noVals = 0;
    unsigned int smoothInterval;
    unsigned int pollTimeSec;
    double sum = 0.0;
    double eps;
    bool first = true;
    double meanVal = 0;
    double lastVal = SENSOR_VALUE_INVALID;
    unsigned long last;

    sensorprocessor(unsigned int smoothInterval = 5, int unsigned pollTimeSec = 60,
                    double eps = 0.1)
        : smoothInterval{smoothInterval}, pollTimeSec{pollTimeSec}, eps{eps} {
        /*! Creates a new sensorprocessor
        @param smoothInterval The size of the interval of sensor value history
        that are being averaged using exponential decay.
        @param pollTimeSec If this is !=0, a valid sensor reading is generated
        at least every pollTimeSec, regardless of value changes.
        @param eps The minimal change required for the smoothed sensor value in
        order to create a new valid reading. Useful for supressing small
        fluctuations.
        */
        reset();
    }

    bool filter(double *pvalue) {
        /*! The sensorprocessor filter function. (double float version)
        @param *pvalue the current raw sensor reading. The filter function uses
        exponential smoothing to filter the value and, if a valid new value is
        available, changes *pvalue.
        @return on true, *pvalue contains a new, smoothed valid sensor reading.
        A new reading is generated by either pollTimeSec (!=0) seconds have been
        passed, or the smoothed value has changed more than espilon eps. A
        return value of false indicates, that no new sensor reading is
        available.
        */
        meanVal = (meanVal * noVals + (*pvalue)) / (noVals + 1);
        if (noVals < smoothInterval) {
            ++noVals;
        }
        double delta = lastVal - meanVal;
        if (delta < 0.0) {
            delta = (-1.0) * delta;
        }
        if (delta > eps || first) {
            first = false;
            lastVal = meanVal;
            *pvalue = meanVal;
            last = millis();
            return true;
        } else {
            if (pollTimeSec != 0) {
                if (timeDiff(last, millis()) > pollTimeSec * 1000L) {
                    *pvalue = meanVal;
                    last = millis();
                    lastVal = meanVal;
                    return true;
                }
            }
        }
        return false;
    }

    bool filter(long *plvalue) {
        /*! The sensorprocessor filter function. (long integer version)
        @param *plvalue the current raw sensor reading. The filter function uses
        exponential smoothing to filter the value and, if a valid new value is
        available, changes *plvalue.
        @return on true, *plvalue contains a new, smoothed valid sensor reading.
        A new reading is generated by either pollTimeSec (!=0) seconds have been
        passed, or the smoothed value has changed more than espilon eps. A
        return value of false indicates, that no new sensor reading is
        available.
        */
        double tval = (double)*plvalue;
        bool ret = filter(&tval);
        if (ret) {
            *plvalue = (long)tval;
        }
        return ret;
    }

    void reset() {
        /*! Delete the filter history */
        noVals = 0;
        sum = 0.0;
        first = true;
        meanVal = 0;
        lastVal = SENSOR_VALUE_INVALID;
        last = millis();
    }

    void update(unsigned int _smoothInterval = 5, int unsigned _pollTimeSec = 60,
                    double _eps = 0.1) {
        /*! Update filter parameters and reset.
         *
         * Note: this is equivalent of recreating a new instance.
         *
        @param _smoothInterval The size of the interval of sensor value history
        that are being averaged using exponential decay.
        @param _pollTimeSec If this is !=0, a valid sensor reading is generated
        at least every pollTimeSec, regardless of value changes.
        @param _eps The minimal change required for the smoothed sensor value in
        order to create a new valid reading. Useful for supressing small
        fluctuations.
        */
        smoothInterval = _smoothInterval;
        pollTimeSec = _pollTimeSec;
        eps = _eps;
        reset();
    }
};

/*!  \brief muwerk numericFunction class
 *
 * numericFunktion approximates arbitrary values x of a function f(x) defined
 * by a number points (x,y) using linear approximation to nearest neighbour-points.
 *
 * Example:

~~~{.cpp}
    // Define a numeric function for (0,9), (1,3), (2,2.8), (3,1)
    const float cx[] = {0., 1., 2., 3.}, cy[] = {9, 3, 2.8, 1};
    // generate model, extrapolate for x outside of (0...3)
    ustd::numericFunction<float> f(cx, cy, sizeof(cx) / sizeof(float), true);

    for (float x = -1.0; x < 5.0; x+=0.1) {
        printf("%f->%f\n", x, f(x));
    }

    // generates:
    ...
    -0.300000 -> 10.800000
    -0.200000 -> 10.200000
    -0.100000 -> 9.600000
    0.000000 -> 9.000000
    0.100000 -> 8.400000
    0.200000 -> 7.800000
    0.300000 -> 7.200000
    ...
    0.800000 -> 4.200000
    0.900000 -> 3.600000
    1.000000 -> 3.000000
    1.100000 -> 2.980000
    1.200000 -> 2.960000
    1.300000 -> 2.940000
    ...
    1.800000 -> 2.840000
    1.900000 -> 2.820000
    2.000000 -> 2.800000
    2.100000 -> 2.620000
    2.200000 -> 2.440000
    ...
    2.800000 -> 1.360000
    2.900000 -> 1.180000
    3.000000 -> 1.000000
    3.100000 -> 0.940000
    3.200000 -> 0.880000
    ...
~~~
*/
template <typename T_FLOAT> class numericFunction {
  public:
    ustd::array<T_FLOAT> x, y;
    T_FLOAT minX, minY, maxX, maxY;
    unsigned int len;
    bool dir;
    bool extrapolate;
    numericFunction(const T_FLOAT px[], const T_FLOAT py[], unsigned int count,
                    bool _extrapolate = false) {
        /*! Instatiate a numericFunction with point px and py.

        @param px array of length count of x-values.
        @param py corresponding array of y-values, f(px[i])=py[i]
        @param count array member count of both px and py
        @param _extrapolate false: if px is ouside of the defined points, x<min(px) gives py[0],
                            x>max(px) gives py[count-1], on true linear approximation is used.
        */

        extrapolate = _extrapolate;
        len = 0;
        for (unsigned int i = 0; i < count; i++) {
            if (i > 0) {
                // enforce strict monotony:
                if (px[i] <= x[len - 1])
                    continue;
                if (py[i] == y[len - 1])
                    continue;

                if (len == 1)
                    dir = (py[i] > y[len - 1]);
                if ((py[i] > y[len - 1]) != dir)
                    continue;
            }
            x[len] = px[i];
            y[len] = py[i];
            if (i == 0 || minX > x[len])
                minX = x[len];
            if (i == 0 || minY > y[len])
                minY = y[len];
            if (i == 0 || maxX < x[len])
                maxX = x[len];
            if (i == 0 || maxY < y[len])
                maxY = y[len];
            ++len;
        }
    }

    static T_FLOAT min(ustd::array<T_FLOAT> ar) {
        /*! get minimum value of array ar
        @param ar ustd::array
        */
        T_FLOAT minVal = 0.0;
        for (unsigned int i = 0; i < ar.length(); i++) {
            if (i == 0 || ar[i] < minVal)
                minVal = ar[i];
        }
        return minVal;
    }

    static T_FLOAT max(ustd::array<T_FLOAT> ar) {
        /*! get maximum value of array ar
        @param ar ustd::array
        */
        T_FLOAT maxVal = 0.0;
        for (unsigned int i = 0; i < ar.length(); i++) {
            if (i == 0 || ar[i] > maxVal)
                maxVal = ar[i];
        }
        return maxVal;
    }

    static void rescale(ustd::array<T_FLOAT> *par, T_FLOAT *pminX, T_FLOAT *pmaxX, T_FLOAT newMin,
                        T_FLOAT newMax) {
        /*! inplace rescale an array so that it conforms to newMin and newMax

        Note: current minimum and maximum must be given to pminX and pmaxX (see \ref min(), \ref
        max() ).

        @param par point to ustd::array to be rescaled
        @param pminX pointer to mimimum value of array-members in ar, will be overwriten with new
        actual minimum.
        @param pmaxX pointer to maximum value of array-members in ar, will be overwriten with new
        actual maximum.
        @param newMin The entire array ar is transformed linearily so that the new minimum is newMin
        @param newMax The entire array ar is transformed linearily so that the new maximum is
        newMax.
        */
        T_FLOAT dx;
        T_FLOAT newMinX, newMaxX;
        unsigned int len = (*par).length();
        if (len < 2 || *pminX == *pmaxX)
            dx = 1;
        else
            dx = (*pmaxX - *pminX);
        T_FLOAT ndx = newMax - newMin;
        for (unsigned int i = 0; i < len; i++) {
            T_FLOAT xi = (*par)[i];
            (*par)[i] = (xi - *pminX) / dx * ndx + newMin;
            if (i == 0 || newMinX > xi)
                newMinX = xi;
            if (i == 0 || newMaxX < xi)
                newMaxX = xi;
        }
        *pminX = newMinX;
        *pmaxX = newMaxX;
    }

    void rescaleX(T_FLOAT newMin, T_FLOAT newMax) {
        /*! Rescale x-axis linearily

        @param newMin new start of x-values
        @param newMax new end of x-values
        */
        rescale(&x, &minX, &maxX, newMin, newMax);
    }
    void rescaleY(T_FLOAT newMin, T_FLOAT newMax) {
        /*! Rescale y-axis linearily

        @param newMin new start of y-values
        @param newMax new end of y-values
        */
        rescale(&y, &minY, &maxY, newMin, newMax);
    }

    static int linsearch(ustd::array<T_FLOAT> &ar, T_FLOAT x) {
        /*! Get largest index element in ar that is smaller than x using bineary search.

        Note: ar must be strictly monotone rising. If x is outside of minX and maxX,
        the nearest array index (either 0 or length-1) is given.

        @param ar ustd::array
        @param x value to be searched
        */
        int a = 0, b = ar.length() - 1, n;
        while (b - a > 1) {
            n = (a + b) / 2;
            if (ar[n] == x)
                return n;
            if (x > ar[n])
                a = n;
            else
                b = n;
        }
        return a;
    }

    T_FLOAT interpol(T_FLOAT xi) {
        /*! Get interpolated value at point f(xi)
        @param xi Value of x used to interpole f(x)
        */
        T_FLOAT dx1, dx2, dy;
        if (len == 0)
            return 0.0;
        if (len == 1)
            return x[0];
        if (xi < minX) {
            if (!extrapolate)
                return y[0];
            else {
                dx1 = x[0] - xi;
                dx2 = x[1] - x[0];
                dy = y[1] - y[0];
                return y[0] - dy / dx2 * dx1;
            }
        }
        if (xi > maxX) {
            if (!extrapolate)
                return y[len - 1];
            else {
                dx1 = x[len - 1] - x[len];
                dx2 = xi - x[len - 1];
                dy = y[len - 1] - y[len - 2];
                return y[len - 1] + dy / dx1 * dx2;
            }
        }
        unsigned int n = linsearch(x, xi);
        if (n >= len - 1)
            return y[len - 1];
        dx1 = x[n] - x[n + 1];
        dx2 = xi - x[n];
        dy = y[n + 1] - y[n];
        float yi = y[n] - dy / dx1 * dx2;
        return yi;
    }

    T_FLOAT operator()(T_FLOAT x) {
        /*! interpolate f(x), uses \ref interpol()
        @param x value to be approximated by f(x).
        */
        return interpol(x);
    }
};

}  // namespace ustd