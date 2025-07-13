1* Search for:
#include "PythonPlayer.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "PythonOfflineshop.h"
#endif

3* Search for:
		CPythonSkill				m_pySkill;

4* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
		CPythonOfflineshop m_pyOfflineshop;
#endif