all: libQtSimpleRpc.so

libQtSimpleRpc.so: $(ls qtsimplerpc/*.h qtsimplerpc/*.cpp qtsimplerpc/*.pro) lib
	@cd lib && qmake ../qtsimplerpc
	@cd lib && make

lib:
	@mkdir lib

