#include "main_incl.h"

#include "./mc_scverify.h"

////// IN 16
  template<int K>
  void CCS_BLOCK(fpaRed_ol_82_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_82_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_82_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_82_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_44)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_44<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_28_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_28_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_28_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_28_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_242_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_242_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_242_area_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_242_area_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_422_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_422_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_422_area_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_422_area_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_224_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_224_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_224_area_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_224_area_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2222_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_4x2_area<K>(vec);
    out = ffp;
  }

////// IN 32
  template<int K>
  void CCS_BLOCK(fpaRed_ol_162_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_162_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_162_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_162_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_216_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_216_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_216_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_216_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_282_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_282_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_282_area_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_282_area_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_282_lat_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_282_lat_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_282_lat_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_282_lat_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_244_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_244_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_244_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_244_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_442_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_442_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_442_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_442_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_822_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_822_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_822_area_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_822_area_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_424_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_424_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_424_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_424_lat<K>(vec);
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
  void CCS_BLOCK(fpaRed_ol_228_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_228_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2224_2xarea_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2224_2xarea_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2224_2xarea_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2224_2xarea_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2242_2xarea_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2242_2xarea_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2242_2xarea_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2242_2xarea_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2422_2xarea_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2422_2xarea_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2422_2xarea_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2422_2xarea_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_4222_2xarea_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_4222_2xarea_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_4222_2xarea_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_4222_2xarea_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_5x2_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_5x2_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_5x2_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_5x2_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce<K>(vec);
    out = ffp;
  }

////// IN 64
  template<int K>
  void CCS_BLOCK(fpaRed_ol_322_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_322_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_322_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_322_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_232_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_232_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_232_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_232_lat<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2162_area_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2162_area_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2162_area_lat)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2162_area_lat<K>(vec);
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
  void CCS_BLOCK(fpaRed_ol_284_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_284_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_248_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_248_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_428_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_428_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_482_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_428_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_842_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_842_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_824_area)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_824_area<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_444)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_444<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2822_3xarea)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2822_3xarea<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_2228_3xarea)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_2228_3xarea<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_24222_4xarea)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_24222_4xarea<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_22224_4xarea)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_22224_4xarea<K>(vec);
    out = ffp;
  }
  template<int K>
  void CCS_BLOCK(fpaRed_ol_6x2_6xarea)(T vec[K], T &out) {
    T ffp(0); ffp.fpaReduce_online_6x2_area<K>(vec);
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
