#include <iostream>
#include <string>
#include <cassert>
#include <iomanip>
#include <cstdlib>

using std::cout;
using std::endl;
using std::string;
using std::setprecision;

FILE *FOPEN(const char fname[], char const flag[]) {
	FILE *fd = fopen(fname, flag);
	if (fd == NULL) {
		fprintf(stderr, "Unable to open file %s with flag %s\n", fname, flag);
		exit(-1);
	}
	return fd;
}

void Usage(const char *program_name) {
  fprintf(stderr, "Usage: %s <htk_file1> <htk_file2>...\n", program_name);
}

template<class _Tp>
class DataMatrix {
  public:
  DataMatrix() : data(NULL), size(0) {}
  ~DataMatrix() {
    if (data) delete [] data;
  }
  void Allocate(unsigned required) {
    if (required > size) {
      if (size == 0) size = 1;
      while (required > size) {
        size *= 2;
      }
      if (data) delete [] data;
      data = new _Tp[size];
    }
  }
  void Load(FILE *fd, unsigned T, unsigned F) {
    unsigned required = T * F;
    Allocate(required);
    assert(fread(data, sizeof(_Tp), required, fd) == required);
  }
  void Dump(unsigned T, unsigned F) {
    unsigned required = T * F;
    for (unsigned i = 0, f = 0, t = 0; i < required; ++i) {
      if (f == 0)
        cout << t << ":";
      cout << setprecision(2) << "  " << data[i];
      f = (f + 1) % F;
      if (f == 0){
        ++t;
        cout << endl;
      }
    }
  }
  private:
  _Tp *data;
  unsigned size;
};

int main(const int argc, const char **argv) {
  if (argc < 2) {
    Usage(argv[0]);
    return 1;
  }
  FILE *fd;
  /* A sample is a numDim vector:
   * sampSize = numDim * valSize (unit = bytes)
   * numSamp = number of time frames
   */
  int numSamp = 0, sampSize = 0, numDim = 0, valSize = 0;
  int parmKind = 0, sampPeriod = 0;
  int sizeFloat = sizeof(float);
  int sizeInt = sizeof(int);
  DataMatrix<float> float_matrix;
  DataMatrix<int> int_matrix;

  string str_pkind;

  for (int i = 1; i < argc; ++i) {
    printf("FILE = %s\n", argv[i]);
    fd = FOPEN(argv[i], "r");
    assert(fread(&numSamp, 4, 1, fd) == 1);
    assert(fread(&sampPeriod, 4, 1, fd) == 1);
    assert(fread(&sampSize, 2, 1, fd) == 1);
    assert(fread(&parmKind, 2, 1, fd) == 1);

    valSize = (parmKind & 02000) ? 2 : 4;
    numDim = sampSize / valSize;

    switch (parmKind & 0x3f) {/*{{{*/
      case 0:
        str_pkind = "WAVEFORM";
        break;
      case 1:
        str_pkind = "LPC";
        break;
      case 2:
        str_pkind = "LPREFC";
        break;
      case 3:
        str_pkind = "LPCEPSTRA";
        break;
      case 4:
        str_pkind = "LPDELCEP";
        break;
      case 5:
        str_pkind = "IREFC";
        break;
      case 6:
        str_pkind = "MFCC";
        break;
      case 7:
        str_pkind = "FBANK";
        break;
      case 8:
        str_pkind = "MELSPEC";
        break;
      case 9:
        str_pkind = "USER";
        break;
      case 10:
        str_pkind = "DISCRETE";
        break;
      default:
        str_pkind = "UNKNOWN";
    }/*}}}*/
    if (parmKind & 000100) {/*{{{*/
      str_pkind += "_E";
    }
    if (parmKind & 000200) {
      str_pkind += "_N";
    }
    if (parmKind & 000400) {
      str_pkind += "_D";
    }
    if (parmKind & 001000) {
      str_pkind += "_A";
    }
    if (parmKind & 002000) {
      str_pkind += "_C";
    }
    if (parmKind & 004000) {
      str_pkind += "_Z";
    }
    if (parmKind & 010000) {
      str_pkind += "_K";
    }
    if (parmKind & 020000) {
      str_pkind += "_0";
    }/*}}}*/
    printf("  numSamp    = %d\n", numSamp);
    printf("  sampPeriod = %d\n", sampPeriod);
    printf("  sampSize   = %d\n", sampSize);
    printf("  numDim     = %d\n", numDim);
    printf("  pramKind   = %o (%s)\n", parmKind, str_pkind.c_str());
    printf("  DATA:\n");
    /* Uncompressed type */
    if (valSize == 4) {
      assert(sizeFloat == valSize);
      float_matrix.Load(fd, numSamp, numDim);
      float_matrix.Dump(numSamp, numDim);
    }
    /* Compressed type */
    else if (sampSize == 2) {
      assert(sizeInt == sampSize);
      int_matrix.Load(fd, numSamp, numDim);
      int_matrix.Dump(numSamp, numDim);
    }
    fclose(fd);
  }
  return 0;
}
