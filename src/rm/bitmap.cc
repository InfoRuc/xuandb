#include "rm.h"
#include <cmath>
#include <cstring>
#include <cassert>

BitMap::BitMap(int num_of_bits): size(num_of_bits)
{
  buffer = new char[this->numChars()];
  // zero out to avoid valgrind warnings.
  memset((void*)buffer, 0, this->numChars());
  this->reset();
}

BitMap::BitMap(char * buf, int num_of_bits): size(num_of_bits)
{
  buffer = new char[this->numChars()];
  memcpy(buffer, buf, this->numChars());
}

int BitMap::toCharBuf(char * b, int len) const //copy content to char buffer -
{
  assert(b != NULL && len == this->numChars());
  memcpy((void*)b, buffer, len);
  return 0;
}

BitMap::~BitMap()
{
  delete [] buffer;
}

int BitMap::numChars() const
{
  int numChars = (size / 8);
  if((size % 8) != 0)
    numChars++;
  return numChars;
}

void BitMap::reset()
{
  for( unsigned int i = 0; i < size; i++) {
    BitMap::reset(i);
  }
}

void BitMap::reset(unsigned int bit_number)
{
  assert(bit_number <= (size - 1));
  int byte = bit_number/8;
  int offset = bit_number%8;
  
  buffer[byte] &= ~(1 << offset);
}

void BitMap::set(unsigned int bit_number)
{
  assert(bit_number <= size - 1);
  int byte = bit_number/8;
  int offset = bit_number%8;

  buffer[byte] |= (1 << offset);
}

void BitMap::set()
{
  for( unsigned int i = 0; i < size; i++) {
    BitMap::set(i);
  }
}

// void BitMap::flip(unsigned int bit_number)
// {
//   assert(bit_number <= size - 1);
//   int byte = bit_number/8;
//   int offset = bit_number%8;

//   buffer[byte] ^= (1 << offset);
// }

bool BitMap::test(unsigned int bit_number) const
{
  assert(bit_number <= size - 1);
  int byte = bit_number/8;
  int offset = bit_number%8;

  return buffer[byte] & (1 << offset);
}


ostream& operator <<(ostream & os, const BitMap& b)
{
  os << "[";
  for(int i=0; i < b.getSize(); i++)
  {
    if( i % 8 == 0 && i != 0 )
      os << ".";
    os << (b.test(i) ? 1 : 0);
  }
  os << "]";
  return os;
}
