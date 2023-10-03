#include "writerwrapper.h"

#include "tlv_define.h"

#include "base/kaldi-common.h"
#include "util/common-utils.h"

using namespace kaldi;

namespace utils {

void write_feats(std::string output_wspecifier,const Matrix<BaseFloat>& features, std::string utt)
{
    try
    {
        BaseFloatMatrixWriter kaldi_writer;  // typedef to TableWriter<something>.
        if (!kaldi_writer.Open(output_wspecifier))
        {
            tlv_log("write_feats:Could not initialize output with wspecifier:%s",output_wspecifier.c_str());
            return;
        }

        std::string key = "default_key";
        if(utt.size() > 0)
        {
            key = utt;
        }
        kaldi_writer.Write(key, features);
        kaldi_writer.Flush();
        kaldi_writer.Close();
    }
    catch(...)
    {
        tlv_log("write_feats:catch exception");
    }
}

void write_matrix(std::string matrix_wspecifier,
                 const Matrix<BaseFloat>& matrix,
                 std::string utt)
{
    try
    {
        std::string key = "default_key";
        if(utt.size() > 0)
        {

            key = utt;
        }
        BaseFloatMatrixWriter matrix_writer(matrix_wspecifier);
        matrix_writer.Write(key, matrix);
        matrix_writer.Flush();
        matrix_writer.Close();
    }
    catch(...)
    {
        tlv_log("write_matrix:catch exception");
    }
}

void write_words(std::string words_wspecifier,
                  const std::vector<int32>& words,
                  std::string utt)
{
    try
    {
        std::string key = "default_key";
        if(utt.size() > 0)
        {

            key = utt;
        }
        Int32VectorWriter words_writer(words_wspecifier);
        words_writer.Write(key, words);
        words_writer.Flush();
        words_writer.Close();
    }
    catch(...)
    {
        tlv_log("write_words:catch exception");
    }
}

void write_single_lattice(std::string wspecifier,
                 const kaldi::Lattice& lat,
                 std::string utt, bool flush)
{
    try
    {
        std::string key = "default_key";
        if(utt.size() > 0)
        {

            key = utt;
        }

        LatticeWriter lattice_writer(wspecifier);
        lattice_writer.Write(key, lat);
        if(flush)
        {
            lattice_writer.Flush();
        }
    }
    catch(...)
    {
        tlv_log("write_single_lattice:catch exception");
    }
}

/*
 * desc  : write_single_clattice
 */
void write_single_clattice(std::string wspecifier,
                 const kaldi::CompactLattice& clat,
                 std::string utt, bool flush)
{
    try
    {
        std::string key = "default_key";
        if(utt.size() > 0)
        {

            key = utt;
        }

        CompactLatticeWriter lattice_writer(wspecifier);
        lattice_writer.Write(key, clat);
        if(flush)
        {
            lattice_writer.Flush();
        }
    }
    catch(...)
    {
        tlv_log("write_single_clattice:catch exception");
    }
}

}//namespace utils
