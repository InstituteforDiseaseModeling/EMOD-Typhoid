#include <Python.h>

#include "MathFunctions.h"
#include "Environment.h"
#include "RANDOM.h"

Kernel::Probability * _instance = Kernel::Probability::getInstance();

void
pyMathFuncInit()
{
    if( Environment::getInstance()->RNG == nullptr )
    {
        unsigned int randomseed[2];
        randomseed[0] = 0;
        randomseed[1] = 0;
        auto rng = new PSEUDO_DES(*reinterpret_cast<uint32_t*>(randomseed));
        const_cast<Environment*>(Environment::getInstance())->RNG = rng;
    }
}

static PyObject*
getFixedDraw(PyObject* self, PyObject* args)
{
    float value;

    /*if (!PyArg_ParseTuple(args, "f", &increment))
        return NULL;*/

    value = _instance->fromDistribution( Kernel::DistributionFunction::FIXED_DURATION, 1 );

    return Py_BuildValue("f", value);
}

static PyObject*
getUniformDraw(PyObject* self, PyObject* args)
{
    float left, right;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &left, &right))
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::UNIFORM_DURATION, left, right );

    return Py_BuildValue("f", value);
}

static PyObject*
getGaussianDraw(PyObject* self, PyObject* args)
{
    float mean, stddev;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &mean, &stddev))
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::GAUSSIAN_DURATION, mean, stddev );

    return Py_BuildValue("f", value);
}

static PyObject*
getExponentialDraw(PyObject* self, PyObject* args)
{
    float param;
    pyMathFuncInit();

    if( !PyArg_ParseTuple(args, "f", &param ) )
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::EXPONENTIAL_DURATION, param );

    return Py_BuildValue("f", value);
}

static PyObject*
getPoissonDraw(PyObject* self, PyObject* args)
{
    float param;
    pyMathFuncInit();

    if( !PyArg_ParseTuple(args, "f", &param ) )
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::POISSON_DURATION, param );

    return Py_BuildValue("f", value);
}

static PyObject*
getLogNormalDraw(PyObject* self, PyObject* args)
{
    float param1, param2;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &param1, &param2))
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::LOG_NORMAL_DURATION, param1, param2 );

    return Py_BuildValue("f", value);
}

static PyObject*
getBimodalDraw(PyObject* self, PyObject* args)
{
    float param1, param2;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &param1, &param2))
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::BIMODAL_DURATION, param1, param2 );

    return Py_BuildValue("f", value);
}

static PyObject*
getWeibullDraw(PyObject* self, PyObject* args)
{
    float param1, param2;
    pyMathFuncInit();

    if (!PyArg_ParseTuple(args, "ff", &param1, &param2))
        return NULL;

    float value = _instance->fromDistribution( Kernel::DistributionFunction::WEIBULL_DURATION, param1, param2 );

    return Py_BuildValue("f", value);
}

static PyMethodDef DtkMathFuncMethods[] =
{
     {"get_fixed_draw", getFixedDraw, METH_VARARGS, "Draw from FIXED distribution."},
     {"get_uniform_draw", getUniformDraw, METH_VARARGS, "Draw from UNIFORM distribution."},
     {"get_gaussian_draw", getGaussianDraw, METH_VARARGS, "Draw from GAUSSIAN distribution."},
     {"get_exponential_draw", getExponentialDraw, METH_VARARGS, "Draw from EXPONENTIAL distribution."},
     {"get_poisson_draw", getPoissonDraw, METH_VARARGS, "Draw from POISSON distribution."},
     {"get_lognormal_draw", getLogNormalDraw, METH_VARARGS, "Draw from LOGNORMAL distribution."},
     {"get_bimodal_draw", getBimodalDraw, METH_VARARGS, "Draw from BIMODAL distribution."},
     {"get_weibull_draw", getWeibullDraw, METH_VARARGS, "Draw from WEIBULL distribution."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_mathfunc(void)
{
     (void) Py_InitModule("dtk_mathfunc", DtkMathFuncMethods);
}
