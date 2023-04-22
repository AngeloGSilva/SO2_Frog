#include <windows.h>
#include "SharedMemory.h"

double factor = 1;

double applyfactor(double v)
{
	return factor * v;
}