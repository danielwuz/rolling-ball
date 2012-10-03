#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

struct point {
	float x;
	float y;
	float z;
};

typedef point triangle[3];

triangle** data;

int data_count = -1;

void display(void);

/*----------
file_in(): file input function. Modify here.
------------*/
void file_in(void)
{
	char* filePath = "sphere.8";
	ifstream file(filePath);
	if(file.fail()){
		cout <<"Cannot open file "<<filePath<<endl;
		exit(0);
	}

	file >> data_count;
	cout <<"Total number of record: "<<data_count<<endl;
	//dynamic memory arrangement
	data = new triangle*[data_count];
	int index = 0;
	while(!file.eof()){
		int v_num;
		file >> v_num;
		//为什么这个tri变量在每次循环中地址都是一样的？
		triangle tri;
		for(int i=0; i<v_num; i++){
			file >> tri[i].x >> tri[i].y >> tri[i].z;
		}
		data[index] = &tri;
		index++;
	}
}


void main(int argc, char **argv)
{
	file_in();
	display();
}

void display(void)
{
	for(int i=0;i<data_count;i++){
		triangle* p = data[i];

		cout << (*p)[0].x <<" "<<(*p)[0].y <<" "<<(*p)[0].z<< endl;
	}
}
