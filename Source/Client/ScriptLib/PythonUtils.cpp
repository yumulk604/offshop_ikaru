1* Search for:
bool PyTuple_GetLong(PyObject* poArgs, int pos, long* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsLong(poItem);
	return true;
}

2* Add below:
bool PyTuple_GetLongLong(PyObject* poArgs, int pos, long long* ret)
{
	if (pos >= PyTuple_Size(poArgs))
	{
		return false;
	}

	PyObject* poItem = PyTuple_GetItem (poArgs, pos);

	if (!poItem)
	{
		return false;
	}

	*ret = PyLong_AsLongLong(poItem);
	return true;
}

3* Search for:
#define PyLong_AsUnsignedLong PyLong_AsUnsignedLongLong

4* comment like that:
// #define PyLong_AsUnsignedLong PyLong_AsUnsignedLongLong
