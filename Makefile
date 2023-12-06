.PHONY: 1 2 3

TEST ?= test_00

1:
	g++ -std=c++17 project_01_01/main.cpp -o project_01_01/main.exe
	project_01_01/main.exe project_01_01/$(TEST).inp test.out
2:
	g++ -std=c++17 project_01_02/main.cpp -o project_01_02/main.exe
	project_01_02/main.exe project_01_02/$(TEST).inp test.out
3:
	g++ -std=c++17 project_01_03/main.cpp -o project_01_03/main.exe
	project_01_03/main.exe project_01_03/$(TEST).inp test.out

test1: 1
	py test.py project_01_01
	
test2: 2
	py test.py project_01_02

test3: 3
	py test.py project_01_03
