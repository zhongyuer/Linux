#定义头文件路径
INCPATH=`pwd`/../include/
SQLINCPATH=`pwd`/../../sqlite/install/include

#定义库文件路径
LIBPATH=`pwd`/../lib/
SQLLIBPATH=`pwd`/../../sqlite/install/lib

INC+=-I ${INCPATH} 
SQLINC+=-I ${SQLINCPATH}

LIB+=-L ${LIBPATH}
SQLLIB+=-L ${SQLLIBPATH}

CC=gcc

all:
	${CC} client.c -o client ${INC} ${LIB} ${SQLINC} ${SQLLIB} -lproject -lsqlite3 

clean:
	rm cliDB.db
	rm client
	rm client.log

client_run:
	export LD_LIBRARY_PATH=/home/iot24/zhongyue/project/client/lib/ && ./client -i127.0.0.1 -p8789 -t5
	

