linux:
	swig -c++ -python bigNumber.i
	g++ -fPIC -c bigNumber.cpp
	g++ -fPIC -c bigNumber_wrap.cxx -I/usr/include/python2.7
	g++ -shared bigNumber.o bigNumber_wrap.o -o _bigNumber.so
	rm bigNumber_wrap.o bigNumber_wrap.cxx bigNumber.o
	touch __init__.py
		
windows:
	swig -c++ -python bigNumber.i
	g++ -c bigNumber.cpp
	g++ -c bigNumber_wrap.cxx -IC:\Python27\include
	g++ bigNumber.o bigNumber_wrap.o -Ic:\python27\include -Lc:\python27\libs -lpython27 -shared -o _bigNumber.pyd

clear:
	rm *.py* && rm *.so