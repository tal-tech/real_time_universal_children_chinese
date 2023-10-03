#ifndef WRITER_WRAPPER_H_
#define WRITER_WRAPPER_H_

#include <string>

#include "tlv/kaldi/base/kaldi-types.h"
#include "tlv/kaldi/matrix/kaldi-matrix.h"
#include "tlv/kaldi/lat/kaldi-lattice.h"

namespace utils {

/*
 * desc  : write_feats
 */
void write_feats(std::string output_wspecifier,
                 const kaldi::Matrix<kaldi::BaseFloat>& features,
                 std::string utt="");

/*
 * desc  : write_matrix
 */
void write_matrix(std::string matrix_wspecifier,
                  const kaldi::Matrix<kaldi::BaseFloat>& matrix,
                  std::string utt="");

/*
 * desc  : write_words
 */
void write_words(std::string words_wspecifier,
                 const std::vector<int32>& words,
                 std::string utt="");

/*
 * desc  : write_single_lattice
 * param : wspecifier example: ark,t:out.lat
 * param : utt : the key
 */
void write_single_lattice(std::string wspecifier,
                 const kaldi::Lattice& lat,
                 std::string utt="", bool flush=true);

/*
 * desc  : write_single_clattice
 * param : wspecifier example: ark,t:out.lat
 * param : utt : the key
 */
void write_single_clattice(std::string wspecifier,
                 const kaldi::CompactLattice& clat,
                 std::string utt="", bool flush=true);


} //namespace utils


#endif  // WRITER_WRAPPER_H_
