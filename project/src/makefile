
#定义库名字
LIBNAME=project

#定义库文件路径
LIBPATH=`pwd`/../lib/
#SQLLIBPATH=`pwd`/../../sqlite/install/lib

#定义头文件路径
INCPATH=`pwd`/../include
SQLINCPATH=`pwd`/../../sqlite/install/include

CC=gcc
AR=AR

INC+=-I${INCPATH} -I${SQLINCPATH}
LIB+=-L${LIBPATH}

all:dynamic_lib
	@make clear
	@make install

dynamic_lib:
	${CC} -shared -fPIC *.c -o lib${LIBNAME}.so ${INC}

clear:
	rm -f *.o

install:
	mv -f lib${LIBNAME}.* ${LIBPATH}

uninstall:
	rm -rf ${LIBPATH}/lib${LIBPATH}.*

clean:
	rm -rf lib${LIBNAME}.*
