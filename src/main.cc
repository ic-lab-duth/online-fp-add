#include "main_incl.h"

#include "./mc_scverify.h"

////// IN 16
  template<int K>
  void CCS_BLOCK(fpaRed_ol_82)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_82<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_44)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_44<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_28)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_28<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_242)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_242<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_422)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_422<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_224)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_224<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2222)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2222<K>(vec);
    out = ffp;
  }

////// IN 32
  template<int K>
  void CCS_BLOCK(fpaRed_ol_162)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_162<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_216)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_216<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_282)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_282<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_244)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_244<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_442)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_442<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_822)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_822<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_424)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_424<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_84)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_84<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_48)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_48<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_228)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_228<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2224)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2224<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2242)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2242<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2422)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2422<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_4222)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_4222<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_22222)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_22222<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce<K>(vec);
    out = ffp;
  }

////// IN 64
  template<int K>
  void CCS_BLOCK(fpaRed_ol_322)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_322<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_232)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_232<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2162)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2162<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_88)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_88<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_164)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_164<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_416)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_416<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_284)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_284<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_248)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_248<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_428)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_428<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_482)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_428<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_842)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_842<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_824)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_824<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_444)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_444<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2822)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2822<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2228)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2228<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_24222)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_24222<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_22224)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_22224<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_222222)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_222222<K>(vec);
    out = ffp;
  }

CCS_MAIN(int arc, char** argv) {
  T Vec[input_size];
  T red_ol(0);

  ///@todo: Replace with BERT input
  for (int i = 0; i < input_size; i++) {
    Vec[i] = i;
  }

  design<input_size>(Vec, red_ol);

  #if LOG == 1
    std::cout << "--- Input: \n";
    for(int i = 0; i < input_size; i++) {
      std::cout << Vec[i].to_float() << " \n"[i==(input_size-1)];
    }
    std::cout << MACRO2STR(design) << ": ";
    std::cout << red_ol.exponent << ", " << red_ol.mantissa.to_string(AC_HEX) << "\n";
    std::cout << red_ol.to_float() << "\n";
  #endif

  CCS_RETURN(0);
}
