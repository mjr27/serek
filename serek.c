#include <Python.h>

#include "stringbuilder.c"

static int
_serialize(PyObject *self, stringbuilder_s *sb)
{
#define SINT_COUNT 14
    if (PyBool_Check(self)) {
        if (self == Py_True) {
            stringbuilder_push(sb, strdup("b:1;"));
        } else {
            stringbuilder_push(sb, strdup("b:0;"));
        }
    } else
    if (PyInt_Check(self)) {
        // TODO: error checking
        long res = PyInt_AS_LONG(self);
        char *str_value = (char*)malloc(SINT_COUNT);
        if (str_value == NULL) {
            PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
            return 1;
        }
        int len = snprintf(str_value, SINT_COUNT, "i:%ld;", res);
        if (len < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }
        stringbuilder_push(sb, str_value);
    } else
    if (PyString_Check(self)) {
        char *str_value = PyString_AS_STRING(self);
        long len = PyString_Size(self);
        long allocated_len = sizeof("s:") + 10 + sizeof(":\"") + len + sizeof("\";") + 1;
        char *own_copy = (char*)malloc(allocated_len);
        if (own_copy == NULL) {
            PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
            return 1;
        }

        int wrote = snprintf(own_copy, allocated_len, "s:%ld:\"%s\";", len, str_value);
        if (wrote < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }

        stringbuilder_push(sb, own_copy);
    } else
    if (self == Py_None) {
        stringbuilder_push(sb, strdup("N;"));
    } else
    if (PyFloat_Check(self)) {
        double dbl_value = PyFloat_AS_DOUBLE(self);
        long allocated_len = sizeof("d:") + 100 + sizeof(";");
        char *str_value = (char*)malloc(allocated_len);
        int len = snprintf(str_value, allocated_len, "d:%lf;", dbl_value);
        if (len < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }
        stringbuilder_push(sb, str_value);
    } else
    if (PyList_Check(self)) {
        Py_ssize_t len = PyList_GET_SIZE(self);
        
        long allocated_len = sizeof("a:") + 10 + sizeof(":{") + 1;
        char *data = (char*)malloc(allocated_len);
        if (data == NULL) {
            PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
            return 1;
        }
        int wrote = snprintf(data, allocated_len, "a:%d:{", len);
        if (wrote < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }
        stringbuilder_push(sb, data);

        Py_ssize_t i = 0;
        for (; i < len; i++) {
            PyObject *obj = PyList_GET_ITEM(self, i);
            
            allocated_len = sizeof("i:") + 10 + sizeof(";") + 1;
            char *data = (char*)malloc(allocated_len);
            if (data == NULL) {
                PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
                return 1;
            }
            int wrote = snprintf(data, allocated_len, "i:%ld;", (long)i);
            if (wrote < 0) {
                PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
                return 1;
            }
            stringbuilder_push(sb, data);

            _serialize(obj, sb);
        }

        stringbuilder_push(sb, strdup("}"));
    } else
    if (PyTuple_Check(self)) {
        Py_ssize_t len = PyTuple_GET_SIZE(self);
        
        long allocated_len = sizeof("a:") + 10 + sizeof(":{") + 1;
        char *data = (char*)malloc(allocated_len);
        if (data == NULL) {
            PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
            return 1;
        }
        int wrote = snprintf(data, allocated_len, "a:%d:{", len);
        if (wrote < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }
        stringbuilder_push(sb, data);

        Py_ssize_t i = 0;
        for (; i < len; i++) {
            PyObject *obj = PyTuple_GET_ITEM(self, i);
            
            allocated_len = sizeof("i:") + 10 + sizeof(";") + 1;
            char *data = (char*)malloc(allocated_len);
            if (data == NULL) {
                PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
                return 1;
            }
            int wrote = snprintf(data, allocated_len, "i:%ld;", (long)i);
            if (wrote < 0) {
                PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
                return 1;
            }
            stringbuilder_push(sb, data);

            _serialize(obj, sb);
        }

        stringbuilder_push(sb, strdup("}"));
    } else
    if (PyMapping_Check(self)) {
        int len = PyMapping_Length(self);

        long allocated_len = sizeof("a:") + 10 + sizeof(":{") + 1;
        char *data = (char*)malloc(allocated_len);
        if (data == NULL) {
            PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
            return 1;
        }
        int wrote = snprintf(data, allocated_len, "a:%d:{", len);
        if (wrote < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }
        stringbuilder_push(sb, data);

        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(self, &pos, &key, &value)) {
            _serialize(key, sb);
            _serialize(value, sb);
        }

        stringbuilder_push(sb, strdup("}"));
    } else
    if (PyUnicode_Check(self)) {
        PyObject *result = PyUnicode_AsEncodedString(self, "utf-8", "ignore");
        if (result == NULL) {
            // Error should be already set
            return 1;
        }

        return _serialize(result, sb);
    } else
    if (PyLong_Check(self)) {
        // I think that Long objects don't occur as ofter as other types, thus they are at the end.

        PyObject *as_string = _PyLong_Format(self, 10, 0, 0);
        if (as_string == NULL) {
            // Someone could check if something different may cause NULL result.
            PyErr_SetString(PyExc_MemoryError, "Probably memory error happened.");
            return 1;
        }

        char *str_value = PyString_AS_STRING(as_string);
        long len = PyString_Size(as_string);
        long allocated_len = sizeof("i:") + len + sizeof(";") + 1;
        char *own_copy = (char*)malloc(allocated_len);
        if (own_copy == NULL) {
            PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
            return 1;
        }

        int wrote = snprintf(own_copy, allocated_len, "i:%s;", str_value);
        if (wrote < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Internal error while doing snprintf().");
            return 1;
        }

        stringbuilder_push(sb, own_copy);
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "Serialization of this type is not supported yet.");
        return 1;
    }

    return 0;
}

static inline long
parse_int(char **start)
{
    unsigned long result = 0;
    char minus = 0;
    if (**start == '-') {
        minus = 1;
        (*start)++;
    }

    while ((**start >= '0') && (**start <= '9')) {
        result = (result*10) + (**start - '0');
        (*start)++;
    }

    return minus ? -result : result;
}

static PyObject *
_deserialize(char **srlzd_ref, char *end)
{
#define srlzd (*srlzd_ref)
    switch (*srlzd++) {
        case 'i': {
            if (*srlzd++ != ':') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":\".");
                return NULL;
            }
 
            char *num_end = srlzd;
            while ((*num_end != '\0') && (*num_end != ';'))
                num_end++;

            if (num_end == srlzd) {
                PyErr_SetString(PyExc_ValueError, "Parse error: zero-length number.");
                return NULL;
            }

            char *num_copy = (char*)malloc(num_end - srlzd + 1);
            if (num_copy == NULL) {
                PyErr_SetString(PyExc_MemoryError, "malloc() failed!");
                return NULL;
            }
            memcpy(num_copy, srlzd, num_end - srlzd);
            num_copy[num_end - srlzd] = '\0';

            // PyInt_FromString is so nice that it returns PyLong values when overflows occur.
            PyObject *result = PyInt_FromString(num_copy, NULL, 0);
            if (result == NULL) {
                PyErr_SetString(PyExc_ValueError, "Parse error: couldn't parse integer.");
                free(num_copy);
                return NULL;
            }
            free(num_copy);

            srlzd = num_end;

            if (*srlzd++ != ';') {
                Py_DECREF(result);
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \";\".");
                return NULL;
            }

            return result;
        }
        case 's': {
            if (*srlzd++ != ':') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":\".");
                return NULL;
            }

            long str_len = parse_int(&srlzd);

            if ((*srlzd++ != ':') || (*srlzd++ != '"')) {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":\"\".");
                return NULL;
            }

            if ((srlzd + str_len) > end) {
                // Declared length of string exceeds length of the whole
                // serialized object.
                PyErr_SetString(PyExc_ValueError, "Parse error: string shorter than declared.");
                return NULL;
            }

            char *str_start = srlzd;
            srlzd += str_len;

            if ((*srlzd++ != '"') || (*srlzd++ != ';')) {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \"\";\".");
                return NULL;
            }

            PyObject *result = PyString_FromStringAndSize(str_start, str_len);
            return result;
        }
        case '\0': {
            Py_INCREF(Py_None);
            return Py_None;
        }
        case 'N': {
            if (*srlzd++ != ';') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \";\".");
                return NULL;
            }

            Py_INCREF(Py_None);
            return Py_None;
        }
        case 'b': {
            if (*srlzd++ != ':') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":\".");
                return NULL;
            }

            char value = 0;

            switch (*srlzd++) {
                case '0':
                    value = 0;
                    break;
                case '1':
                    value = 1;
                    break;
                default:
                    /* PHP accepts only 0 and 1 */
                    PyErr_SetString(PyExc_ValueError, "Parse error: incorrect boolean value, expected 0 or 1.");
                    return NULL;
            }
            //char value = (*srlzd++ != '0');

            if (*srlzd++ != ';') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \";\".");
                return NULL;
            }

            if (value) {
                Py_INCREF(Py_True);
                return Py_True;
            } else {
                Py_INCREF(Py_False);
                return Py_False;
            }
        }
        case 'd': {
            if (*srlzd++ != ':') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":\".");
                return NULL;
            }
            
            char *val_end = srlzd;
            while ((val_end < end) && (*val_end != ';'))
                val_end++;

            if (val_end == srlzd) {
                // No value, ';' immediately after 'i' (i;).
                PyErr_SetString(PyExc_ValueError, "Parse error: expected value.");
                return NULL;
            }

            double value = strtod(srlzd, &val_end);
            if ((value == 0.0) && (errno == ERANGE)) {
                PyErr_SetString(PyExc_ValueError, "Parse error: double range exceeded.");
                return NULL;
            }

            srlzd = val_end;

            if (*srlzd++ != ';') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \";\".");
                return NULL;
            }

            /*
            PyObject *result = PyFloat_FromString(srlzd, &val_end);
            if (result == NULL)
                return NULL;*/

            PyObject *result = PyFloat_FromDouble(value);
            if (result == NULL) {
                // Someone could check if something different may cause NULL result.
                PyErr_SetString(PyExc_MemoryError, "Probably memory error happened.");
                return NULL;
            }
            return result;

        }
        case 'a': {
            if (*srlzd++ != ':') {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":\".");
                return NULL;
            }

            long arr_len = parse_int(&srlzd);

            if ((*srlzd++ != ':') || (*srlzd++ != '{')) {
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \":{\".");
                return NULL;
            }

            PyObject *result = PyDict_New();
            if (result == NULL) {
                PyErr_SetString(PyExc_MemoryError, "Probably memory error happened.");
                return NULL;
            }

            while (arr_len--)  {
                PyObject *key = NULL;
                PyObject *value = NULL;

                key = _deserialize(srlzd_ref, end);
                if (key == NULL) {
                    Py_DECREF(result);
                    return NULL;
                }
                value = _deserialize(srlzd_ref, end);
                if (value == NULL) {
                    Py_DECREF(key);
                    Py_DECREF(result);
                    return NULL;
                }

                PyDict_SetItem(result, key, value);
                Py_DECREF(key);
                Py_DECREF(value);
            }

            if (*srlzd++ != '}') {
                Py_DECREF(result);
                PyErr_SetString(PyExc_ValueError, "Parse error: expected \"}\".");
                return NULL;
            }

            return result;
        }
        default: {
            PyErr_SetString(PyExc_ValueError, "Parse error: such object type is not supported.");
        }
    }

    return NULL;
}

static PyObject *
serek_deserialize(PyObject *self, PyObject *args)
{
    char *data = NULL;

    if (! PyArg_ParseTuple(args, "s", &data))
        return NULL;

    char *end = data + strlen(data);
    PyObject *result = _deserialize(&data, end);
    if ((result != NULL) && (data < end)) {
        PyErr_SetString(PyExc_ValueError, "Parse error: Garbage data at the end of the serialized string");
        return NULL;
    }

    return result;
}

static PyObject *
serek_serialize(PyObject *self, PyObject *args)
{
    PyObject *data = NULL;

    if (! PyArg_ParseTuple(args, "O", &data))
        return NULL;

    stringbuilder_s *sb = stringbuilder_new();
    if (sb == NULL) {
        return PyErr_NoMemory();
    }

    if (_serialize(data, sb))
        // Returns 1 if failed somehow.
        return NULL;

    char *result = stringbuilder_build(sb);
    stringbuilder_free(sb, 1);
    if (result == NULL) {
        return PyErr_NoMemory();
    }

    PyObject *serialized = Py_BuildValue("s", result);
    free(result);
    return serialized;
}


static PyMethodDef SpamMethods[] = {
    {"serialize",  serek_serialize, METH_VARARGS,
     "Serialize object."},
    {"dumps",  serek_serialize, METH_VARARGS,
     "Serialize object."},
    {"deserialize",  serek_deserialize, METH_VARARGS,
     "Deserialize string."},
    {"loads",  serek_deserialize, METH_VARARGS,
     "Deserialize string."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initserek(void)
{
    (void) Py_InitModule("serek", SpamMethods);
}

