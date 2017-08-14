#include <iostream>
#include <cstdlib>
#include <ctime> 
#include <string>
using namespace std;
//using namespace bdbh;

void Usage(char ** argv)
{
  cerr << "Usage: " << *argv << " length number" << '\n';
  exit(1);
}

int main (int argc,char* argv[]) 
{ 
  if (argc<3)
    {
      Usage(argv);
    }

  timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  srandom(tp.tv_nsec);

  int preferred_length = atoi(argv[1]);
  int nb               = atoi(argv[2]);
  float avg = 0;

  for ( int i=0; i<nb; i++)
    {
      int length =  ((1.0*random())/RAND_MAX) * preferred_length * 2;
      if ( length==0) { length = 1; };
      string name = "";
      for (int j=0; j<length; j++)
	{
	  name += 'a'+ (int)(1.0*random()*26/RAND_MAX);
	}
      cout << name << ' ';
      avg += length;
    }
  //  cout << '\n';
  // cout << avg/nb << '\n';
}


//  string name;
//  for ( int i=0; i<preferred_length*2; i++)
//    {
//      /* random number */
//      float x1 = random()/RAND_MAX*i/preferred_length;
//	 If 
      
  
