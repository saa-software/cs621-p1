#include <fstream>
using namespace std;
#include <iostream>

int 
main (int argc, char *argv[])
{
	int *m_data;
	char *newLine = "\n";
	ifstream file;
	file.open ("/dev/random");

	ofstream ofile;
	ofile.open ("data.txt");
	
	if (file.is_open ())
	{
	  for (int i = 0; i < 6000; ++i)
	  {
	  	m_data = new int [1100];
	  	file.read((char*)m_data, 1100);
	  	ofile.write((char*)&m_data, 1100);
	  	delete [] m_data;
	  }
	}

	ofile.close();
	file.close();
}