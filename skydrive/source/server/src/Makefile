CC = c++
SUFFIX = .cpp
EXE = skydrive_server
SRC = main tools interface sql
COMMON_PATH = ../common/
COMMON_SRC = 
HEAD = `mysql_config --cflags`
LIBS = -L/usr/lib64/mysql -lmysqlclient

.PHONY:all clean

all:$(EXE)

$(EXE):$(foreach N, $(SRC), $N$(SUFFIX)) $(foreach N, $(COMMON_SRC), $N$(SUFFIX))
	$(CC) -Wall $(HEAD) $(LIBS) -o $@ $^ 

clean:
	rm -f *.o $(EXE) log* *.txt