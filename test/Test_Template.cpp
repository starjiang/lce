#include <iostream>

class A
{

};

template<typename T>
class B
{
public:	
	B(){m_poData = new T;}
	bool init(){ return false; }

	static B<T> & getInstance()
	{

		if (NULL == m_pInstance)
		{
			m_pInstance = new B;
		}
		return *m_pInstance;
	}

private:
	T *m_poData;
	static B<T> *m_pInstance;

};

B<A>* B<A>::m_pInstance = NULL;


int main(int argc ,char**argv)
{

	B<A>::getInstance().init();

	B<A> *b = new B<A>;

	return 0;
}