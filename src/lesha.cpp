#include <iostream>

using namespace std;

class RealVec{
  float *array;
  int len;
  public:
  RealVec (int N=0) : array(0), len(0)
  {
    if (N>0) {array = new float[N];}
    if (array) {len = N;}
  }
  RealVec (const RealVec & temp) : array(0), len(0)
  { 
    if (temp.len>0) array = new float [temp.len];
    if ( array ) 
    {
      len=temp.len;
      for (int i=0;i<len;i++) {array[i]=temp.array[i];}
    }
  }
  int size() const { return len; }
  
  float & operator [] (int index)
  { return array[index]; }
  
  float operator [] (int index) const
  { return array[index]; }
  
  ~ RealVec() { delete [] array; }
  
  void insert(float value)
  {
    float * parray = new float[len + 1];
    cout << len + 1 << endl;
    if(parray == NULL)
    {
      exit(1);
    }
    for( int index = 0; index < len; index++)
    {
        parray[index] = array[index];
    }
    len += 1;
    parray[len - 1] = value;
    cout << parray[0] << endl;
    array = parray;
    //delete parray;
    
  }
  
  void erase(int dindex)
  {
    float * parray = new float[len - 1];
    if(parray == NULL)
    {
      exit(1);
    }
    int offset = 0;
    for (int index = 0; index < len; index++)
    {
      
      if(dindex == index)
      {
        offset = 1;
        continue;
      }
      parray[index - offset] = array[index];
    }
    array = parray;
    delete parray;
  }
  
  void sort(bool reversed = false)
  {
    for (int iter = 0; iter < len; iter++)
    {
        for (int i = 0; i < len - 1 - iter; i++)
        {
          bool needSwap;
            if (reversed) {
              needSwap = array[i] < array[i + 1];
            } else {
              needSwap = array[i] > array[i + 1];
            };
            if (needSwap) 
            {
                float temp = array[i];
                array[i] = array[i+1];
                array[i+1] = temp;
            }
        }
    }
  }
  
  void push_at(int nindex, float value)
  {
    float * parray = new float[len + 1];
    if(parray == NULL)
    {
      exit(1);
    }
    for( int index = 0; index < len; index++)
    {
      if(nindex == index)
      {
        parray[index] = value;
        index +=1;
      }
      parray[index] = array[index];
      
    }
    len += 1;
    array = parray;
    delete parray;
  }

};

ostream& operator<<(ostream& os, const RealVec& src)
{ 
  os << "[ ";
  for(int i=0; i<src.size(); i++)
  {
    os << src[i] << " ";
  }
  os << "]";
  return os;
}



int main(){
  RealVec a(5);
  a.insert(1.);
  //a.insert(2.);
  //a.insert(3.);
  cout<<a;
  a.sort();
  
}

