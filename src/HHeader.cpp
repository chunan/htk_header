#include <iostream>
#include <string.h>
#include <cassert>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::setprecision;

FILE *FOPEN(const char fname[], char const flag[]) {
  FILE *fd = fopen(fname, flag);
  if (fd == NULL) {
    cerr << "Unable to open file " << fname << " with flag " << flag << "\n";
    exit(-1);
  }
  return fd;
}

void Usage(const char *program_name) {
  cerr << "Usage: " << program_name << " [options] <htk_file1> <htk_file2>...\n"
       << "  [options]\n"
       << "    -d     Dump data matrix.\n"
       << "    -p int Precision when dump.\n";
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
  void Dump(unsigned T, unsigned F, int precision) {
    unsigned required = T * F;
    for (unsigned i = 0, f = 0, t = 0; i < required; ++i) {
      if (f == 0)
        cout << t << ":";
      cout << setprecision(precision) << "  " << data[i];
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
  bool dumpData = false;
  /* A sample is a numDim vector:
   * sampSize = numDim * valSize (unit = bytes)
   * numSamp = number of time frames
   */
  int32_t numSamp = 0, sampPeriod = 0;
  int16_t sampSize = 0, parmKind = 0;
  int numDim = 0, valSize = 0;
  int sizeFloat = sizeof(float);
  int precision = 2;
  DataMatrix<float> float_matrix;
  DataMatrix<int16_t> int_matrix;

  string str_pkind;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-d") == 0) {
      dumpData = true;
    } else if (strcmp(argv[i], "-p") == 0) {
      assert(i + 1 < argc);
      precision = atoi(argv[i + 1]);
    }
  }

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-d") == 0) {
      continue;
    } else if (strcmp(argv[i], "-p") == 0) {
      ++i;
      continue;
    }
    cout << "FILE = " << argv[i] << "\n";
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
    cout << "  numSamp    = " << numSamp << endl;
    cout << "  sampPeriod = " << sampPeriod << endl;
    cout << "  sampSize   = " << sampSize << endl;
    cout << "  numDim     = " << numDim << endl;
    cout << "  parmKind   = " << parmKind << "(" << str_pkind << ")" << endl;
    if (dumpData) {
      cout << "  DATA:\n";
      /* Uncompressed type */
      if (valSize == 4) {
        assert(sizeFloat == valSize);
        float_matrix.Load(fd, numSamp, numDim);
        float_matrix.Dump(numSamp, numDim, precision);
      }
      /* Compressed type */
      else if (sampSize == 2) {
        int_matrix.Load(fd, numSamp, numDim);
        int_matrix.Dump(numSamp, numDim, precision);
      }
    }
    fclose(fd);
  }
  return 0;
}
