#include "../include/3dglObject.h"

#include <iostream>

using namespace std;
using namespace _3dgl;

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglObject

bool C3dglObject::c_bQuietMode = false;

bool C3dglObject::displayInfo()
{
	string name = getName();
	if (name.empty())
		(m_bStatus ? cout : cerr) << m_info << endl; 
	else
		(m_bStatus ? cout : cerr) << getName() << " " << m_info << endl; 
	return m_bStatus; 
}

