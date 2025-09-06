#include "gdcmReader.h"
#include "gdcmFileDerivation.h"
#include "gdcmAnonymizer.h"
#include "gdcmVersion.h"
#include "gdcmPixmapReader.h"
#include "gdcmPixmapWriter.h"
#include "gdcmWriter.h"
#include "gdcmSystem.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmDataSet.h"
#include "gdcmIconImageGenerator.h"
#include "gdcmAttribute.h"
#include "gdcmSequenceOfItems.h"
#include "gdcmUIDGenerator.h"
#include "gdcmImage.h"
#include "gdcmImageChangeTransferSyntax.h"
#include "gdcmImageApplyLookupTable.h"
#include "gdcmFileDecompressLookupTable.h"
#include "gdcmImageFragmentSplitter.h"
#include "gdcmImageChangePlanarConfiguration.h"
#include "gdcmImageChangePhotometricInterpretation.h"
#include "gdcmFileExplicitFilter.h"
#include "gdcmJPEG2000Codec.h"
#include "gdcmJPEGCodec.h"
#include "gdcmJPEGLSCodec.h"
#include "gdcmSequenceOfFragments.h"

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

// C interface for Go
extern "C" {

// Compression types
typedef enum {
    COMPRESSION_RAW = 0,
    COMPRESSION_JPEG_LOSSY,
    COMPRESSION_JPEG_LOSSLESS,
    COMPRESSION_JPEG2000_LOSSY,
    COMPRESSION_JPEG2000_LOSSLESS,
    COMPRESSION_JPEGLS_LOSSY,
    COMPRESSION_JPEGLS_LOSSLESS,
    COMPRESSION_RLE,
    COMPRESSION_DEFLATED
} compression_type_t;

// Result structure
typedef struct {
    int success;
    char error_message[256];
} gdcm_result_t;

// Function declarations
gdcm_result_t gdcm_convert_image(const char* input_file, const char* output_file,
                                compression_type_t compression, double quality_or_rate);
gdcm_result_t gdcm_convert_transfer_syntax(const char* input_file, const char* output_file,
                                         int explicit_ts, int implicit_ts);
gdcm_result_t gdcm_apply_lut(const char* input_file, const char* output_file, int rgb8);
gdcm_result_t gdcm_remove_tags(const char* input_file, const char* output_file,
                              int remove_private, int remove_retired, int remove_group_length);

}

struct SetSQToUndefined
{
  void operator() (gdcm::DataElement &de) {
    de.SetVLToUndefined();
  }
};

static void PrintVersion()
{
  std::cout << "gdcmconv: gdcm " << gdcm::Version::GetVersion() << " ";
  const char date[] = "$Date$";
  std::cout << date << std::endl;
}

static void PrintLossyWarning()
{
  std::cout << "You have selected a lossy compression transfer syntax." << std::endl;
  std::cout << "This will degrade the quality of your input image, and can." << std::endl;
  std::cout << "impact professional interpretation of the image." << std::endl;
  std::cout << "Do not use if you do not understand the risk." << std::endl;
  std::cout << "WARNING: this mode is very experimental." << std::endl;
}

static void PrintHelp()
{
  PrintVersion();
  std::cout << "Usage: gdcmconv [OPTION] input.dcm output.dcm" << std::endl;
  std::cout << "Convert a DICOM file into another DICOM file.\n";
  std::cout << "Parameter (required):" << std::endl;
  std::cout << "  -i --input      DICOM filename" << std::endl;
  std::cout << "  -o --output     DICOM filename" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -X --explicit            Change Transfer Syntax to explicit." << std::endl;
  std::cout << "  -M --implicit            Change Transfer Syntax to implicit." << std::endl;
  std::cout << "  -U --use-dict            Use dict for VR (only public by default)." << std::endl;
  std::cout << "     --with-private-dict   Use private dict for VR (advanced user only)." << std::endl;
  std::cout << "  -C --check-meta          Check File Meta Information (advanced user only)." << std::endl;
  std::cout << "     --root-uid            Root UID." << std::endl;
  std::cout << "     --remove-gl           Remove group length (deprecated in DICOM 2008)." << std::endl;
  std::cout << "     --remove-private-tags Remove private tags." << std::endl;
  std::cout << "     --remove-retired      Remove retired tags." << std::endl;
  std::cout << "Image only Options:" << std::endl;
  std::cout << "  -l --apply-lut                      Apply LUT (non-standard, advanced user only)." << std::endl;
  std::cout << "  -8 --apply-lut8                     Apply LUT/RGB8 (non-standard, advanced user only)." << std::endl;
  std::cout << "     --decompress-lut                 Decompress LUT (linearize segmented LUT)." << std::endl;
  std::cout << "  -P --photometric-interpretation %s  Change Photometric Interpretation (when possible)." << std::endl;
  std::cout << "  -w --raw                            Decompress image." << std::endl;
  std::cout << "  -d --deflated                       Compress using deflated (gzip)." << std::endl;
  std::cout << "  -J --jpeg                           Compress image in jpeg." << std::endl;
  std::cout << "  -K --j2k                            Compress image in j2k." << std::endl;
  std::cout << "  -L --jpegls                         Compress image in jpeg-ls." << std::endl;
  std::cout << "  -R --rle                            Compress image in rle (lossless only)." << std::endl;
  std::cout << "  -F --force                          Force decompression/merging before recompression/splitting." << std::endl;
  std::cout << "     --generate-icon                  Generate icon." << std::endl;
  std::cout << "     --icon-minmax %d,%d              Min/Max value for icon." << std::endl;
  std::cout << "     --icon-auto-minmax               Automatically compute best Min/Max values for icon." << std::endl;
  std::cout << "     --compress-icon                  Decide whether icon follows main TransferSyntax or remains uncompressed." << std::endl;
  std::cout << "     --planar-configuration [01]      Change planar configuration." << std::endl;
  std::cout << "  -Y --lossy                          Use the lossy (if possible) compressor." << std::endl;
  std::cout << "  -S --split %d                       Write 2D image with multiple fragments (using max size)" << std::endl;
  std::cout << "General Options:" << std::endl;
  std::cout << "  -V --verbose    more verbose (warning+error)." << std::endl;
  std::cout << "  -W --warning    print warning info." << std::endl;
  std::cout << "  -D --debug      print debug info." << std::endl;
  std::cout << "  -E --error      print error info." << std::endl;
  std::cout << "  -h --help       print help." << std::endl;
  std::cout << "  -v --version    print version." << std::endl;
  std::cout << "     --quiet      do not print to stdout." << std::endl;
  std::cout << "JPEG Options:" << std::endl;
  std::cout << "  -q --quality %*f           set quality." << std::endl;
  std::cout << "JPEG-LS Options:" << std::endl;
  std::cout << "  -e --allowed-error %*i             set allowed error." << std::endl;
  std::cout << "J2K Options:" << std::endl;
  std::cout << "  -r --rate    %*f           set rate." << std::endl;
  std::cout << "  -q --quality %*f           set quality." << std::endl;
  std::cout << "  -t --tile %d,%d            set tile size." << std::endl;
  std::cout << "  -n --number-resolution %d  set number of resolution." << std::endl;
  std::cout << "     --irreversible          set irreversible." << std::endl;
  std::cout << "Special Options:" << std::endl;
  std::cout << "  -I --ignore-errors   convert even if file is corrupted (advanced users only, see disclaimers)." << std::endl;
  std::cout << "Env var:" << std::endl;
  std::cout << "  GDCM_ROOT_UID Root UID" << std::endl;
/*
 * Default behavior for root UID is:
 * By default the GDCM one is used
 * If GDCM_ROOT_UID is set, then use this one instead
 * If --root-uid is explicitly set on the command line, it will override any other defined behavior
 */
}

template <typename T>
static size_t readvector(std::vector<T> &v, const char *str)
{
  if( !str ) return 0;
  std::istringstream os( str );
  T f;
  while( os >> f )
    {
    v.push_back( f );
    os.get(); //  == ","
    }
  return v.size();
}

namespace gdcm
{
static bool derives( File & file, const Pixmap& compressed_image )
{
#if 1
  DataSet &ds = file.GetDataSet();

  if( !ds.FindDataElement( Tag(0x0008,0x0016) )
    || ds.GetDataElement( Tag(0x0008,0x0016) ).IsEmpty() )
    {
    return false;
    }
  if( !ds.FindDataElement( Tag(0x0008,0x0018) )
    || ds.GetDataElement( Tag(0x0008,0x0018) ).IsEmpty() )
    {
    return false;
    }
  const DataElement &sopclassuid = ds.GetDataElement( Tag(0x0008,0x0016) );
  const DataElement &sopinstanceuid = ds.GetDataElement( Tag(0x0008,0x0018) );
  // Make sure that const char* pointer will be properly padded with \0 char:
  std::string sopclassuid_str( sopclassuid.GetByteValue()->GetPointer(), sopclassuid.GetByteValue()->GetLength() );
  std::string sopinstanceuid_str( sopinstanceuid.GetByteValue()->GetPointer(), sopinstanceuid.GetByteValue()->GetLength() );
  ds.Remove( Tag(0x8,0x18) );

  FileDerivation fd;
  fd.SetFile( file );
  fd.AddReference( sopclassuid_str.c_str(), sopinstanceuid_str.c_str() );

  // CID 7202 Source Image Purposes of Reference
  // {"DCM",121320,"Uncompressed predecessor"},
  fd.SetPurposeOfReferenceCodeSequenceCodeValue( 121320 );

  // CID 7203 Image Derivation
  // { "DCM",113040,"Lossy Compression" },
  fd.SetDerivationCodeSequenceCodeValue( 113040 );
  fd.SetDerivationDescription( "lossy conversion" );
  if( !fd.Derive() )
    {
    std::cerr << "Sorry could not derive using input info" << std::endl;
    return false;
    }


#else
/*
(0008,2111) ST [Lossy compression with JPEG extended sequential 8 bit, IJG quality... # 102, 1 DerivationDescription
(0008,2112) SQ (Sequence with explicit length #=1)      # 188, 1 SourceImageSequence
  (fffe,e000) na (Item with explicit length #=3)          # 180, 1 Item
    (0008,1150) UI =UltrasoundImageStorage                  #  28, 1 ReferencedSOPClassUID
    (0008,1155) UI [1.2.840.1136190195280574824680000700.3.0.1.19970424140438] #  58, 1 ReferencedSOPInstanceUID
    (0040,a170) SQ (Sequence with explicit length #=1)      #  66, 1 PurposeOfReferenceCodeSequence
      (fffe,e000) na (Item with explicit length #=3)          #  58, 1 Item
        (0008,0100) SH [121320]                                 #   6, 1 CodeValue
        (0008,0102) SH [DCM]                                    #   4, 1 CodingSchemeDesignator
        (0008,0104) LO [Uncompressed predecessor]               #  24, 1 CodeMeaning
      (fffe,e00d) na (ItemDelimitationItem for re-encoding)   #   0, 0 ItemDelimitationItem
    (fffe,e0dd) na (SequenceDelimitationItem for re-encod.) #   0, 0 SequenceDelimitationItem
  (fffe,e00d) na (ItemDelimitationItem for re-encoding)   #   0, 0 ItemDelimitationItem
(fffe,e0dd) na (SequenceDelimitationItem for re-encod.) #   0, 0 SequenceDelimitationItem
*/
    const Tag sisq(0x8,0x2112);
    SequenceOfItems * sqi;
      sqi = new SequenceOfItems;
      DataElement de( sisq);
      de.SetVR( VR::SQ );
      de.SetValue( *sqi );
      de.SetVLToUndefined();

  DataSet &ds = file.GetDataSet();
      ds.Insert( de );
{
    // (0008,0008) CS [ORIGINAL\SECONDARY]                     #  18, 2 ImageType
    gdcm::Attribute<0x0008,0x0008> at3;
    static const gdcm::CSComp values[] = {"DERIVED","SECONDARY"};
    at3.SetValues( values, 2, true ); // true => copy data !
    if( ds.FindDataElement( at3.GetTag() ) )
      {
      const gdcm::DataElement &de = ds.GetDataElement( at3.GetTag() );
      at3.SetFromDataElement( de );
      // Make sure that value #1 is at least 'DERIVED', so override in all cases:
      at3.SetValue( 0, values[0] );
      }
    ds.Replace( at3.GetAsDataElement() );

}
{
    Attribute<0x0008,0x2111> at1;
    at1.SetValue( "lossy conversion" );
    ds.Replace( at1.GetAsDataElement() );
}

    sqi = (SequenceOfItems*)ds.GetDataElement( sisq ).GetSequenceOfItems();
    sqi->SetLengthToUndefined();

    if( !sqi->GetNumberOfItems() )
      {
      Item item; //( Tag(0xfffe,0xe000) );
      item.SetVLToUndefined();
      sqi->AddItem( item );
      }

    Item &item1 = sqi->GetItem(1);
    DataSet &subds = item1.GetNestedDataSet();
/*
    (0008,1150) UI =UltrasoundImageStorage                  #  28, 1 ReferencedSOPClassUID
    (0008,1155) UI [1.2.840.1136190195280574824680000700.3.0.1.19970424140438] #  58, 1 ReferencedSOPInstanceUID
*/
{
    DataElement sopinstanceuid = ds.GetDataElement( Tag(0x0008,0x0016) );
    sopinstanceuid.SetTag( Tag(0x8,0x1150 ) );
    subds.Replace( sopinstanceuid );
    DataElement sopclassuid = ds.GetDataElement( Tag(0x0008,0x0018) );
    sopclassuid.SetTag( Tag(0x8,0x1155 ) );
    subds.Replace( sopclassuid );
    ds.Remove( Tag(0x8,0x18) );
}

    const Tag prcs(0x0040,0xa170);
    if( !subds.FindDataElement( prcs) )
      {
      SequenceOfItems *sqi2 = new SequenceOfItems;
      DataElement de( prcs );
      de.SetVR( VR::SQ );
      de.SetValue( *sqi2 );
      de.SetVLToUndefined();
      subds.Insert( de );
      }

    sqi = (SequenceOfItems*)subds.GetDataElement( prcs ).GetSequenceOfItems();
    sqi->SetLengthToUndefined();

    if( !sqi->GetNumberOfItems() )
      {
      Item item; //( Tag(0xfffe,0xe000) );
      item.SetVLToUndefined();
      sqi->AddItem( item );
      }
    Item &item2 = sqi->GetItem(1);
    DataSet &subds2 = item2.GetNestedDataSet();

/*
        (0008,0100) SH [121320]                                 #   6, 1 CodeValue
        (0008,0102) SH [DCM]                                    #   4, 1 CodingSchemeDesignator
        (0008,0104) LO [Uncompressed predecessor]               #  24, 1 CodeMeaning
*/

    Attribute<0x0008,0x0100> at1;
    at1.SetValue( "121320" );
    subds2.Replace( at1.GetAsDataElement() );
    Attribute<0x0008,0x0102> at2;
    at2.SetValue( "DCM" );
    subds2.Replace( at2.GetAsDataElement() );
    Attribute<0x0008,0x0104> at3;
    at3.SetValue( "Uncompressed predecessor" );
    subds2.Replace( at3.GetAsDataElement() );

/*
(0008,9215) SQ (Sequence with explicit length #=1)      #  98, 1 DerivationCodeSequence
  (fffe,e000) na (Item with explicit length #=3)          #  90, 1 Item
    (0008,0100) SH [121327]                                 #   6, 1 CodeValue
    (0008,0102) SH [DCM]                                    #   4, 1 CodingSchemeDesignator
    (0008,0104) LO [Full fidelity image, uncompressed or lossless compressed] #  56, 1 CodeMeaning
  (fffe,e00d) na (ItemDelimitationItem for re-encoding)   #   0, 0 ItemDelimitationItem
(fffe,e0dd) na (SequenceDelimitationItem for re-encod.) #   0, 0 SequenceDelimitationItem
*/
{
    const Tag sisq(0x8,0x9215);
    SequenceOfItems * sqi;
      sqi = new SequenceOfItems;
      DataElement de( sisq );
      de.SetVR( VR::SQ );
      de.SetValue( *sqi );
      de.SetVLToUndefined();
      ds.Insert( de );
    sqi = (SequenceOfItems*)ds.GetDataElement( sisq ).GetSequenceOfItems();
    sqi->SetLengthToUndefined();

    if( !sqi->GetNumberOfItems() )
      {
      Item item; //( Tag(0xfffe,0xe000) );
      item.SetVLToUndefined();
      sqi->AddItem( item );
      }

    Item &item1 = sqi->GetItem(1);
    DataSet &subds3 = item1.GetNestedDataSet();

    Attribute<0x0008,0x0100> at1;
    at1.SetValue( "121327" );
    subds3.Replace( at1.GetAsDataElement() );
    Attribute<0x0008,0x0102> at2;
    at2.SetValue( "DCM" );
    subds3.Replace( at2.GetAsDataElement() );
    Attribute<0x0008,0x0104> at3;
    at3.SetValue( "Full fidelity image, uncompressed or lossless compressed" );
    subds3.Replace( at3.GetAsDataElement() );
}
#endif

{
  /*
  (0028,2110) CS [01]                                     #   2, 1 LossyImageCompression
  (0028,2112) DS [15.95]                                  #   6, 1 LossyImageCompressionRatio
  (0028,2114) CS [ISO_10918_1]                            #  12, 1 LossyImageCompressionMethod
   */
  const DataElement & pixeldata = compressed_image.GetDataElement();
  size_t len = pixeldata.GetSequenceOfFragments()->ComputeByteLength();
  size_t reflen = compressed_image.GetBufferLength();
  double ratio = (double)reflen / (double)len;
  Attribute<0x0028,0x2110> at1;
  at1.SetValue( "01" );
  ds.Replace( at1.GetAsDataElement() );
  Attribute<0x0028,0x2112> at2;
  at2.SetValues( &ratio, 1);
  ds.Replace( at2.GetAsDataElement() );
  Attribute<0x0028,0x2114> at3;

  // ImageWriter will properly set attribute 0028,2114 (Lossy Image Compression Method)
}

return true;

}
} // end namespace gdcm

namespace {

int change_transfersyntax(const std::string &filename, const std::string &outfilename, int explicitts, int implicit, int deflated, int raw, int changeprivatetags )
{
    if( explicitts && implicit ) return 1; // guard
    if( explicitts && deflated ) return 1; // guard
    if( implicit && deflated ) return 1; // guard
    gdcm::Reader reader;
    reader.SetFileName( filename.c_str() );
    if( !reader.Read() )
      {
      std::cerr << "Could not read: " << filename << std::endl;
      return 1;
      }
    gdcm::MediaStorage ms;
    ms.SetFromFile( reader.GetFile() );
    if( ms == gdcm::MediaStorage::MediaStorageDirectoryStorage )
      {
      std::cerr << "Sorry DICOMDIR is not supported" << std::endl;
      return 1;
      }

    gdcm::Writer writer;
    writer.SetFileName( outfilename.c_str() );
    writer.SetFile( reader.GetFile() );
    gdcm::File & file = writer.GetFile();
    gdcm::FileMetaInformation &fmi = file.GetHeader();

    const gdcm::TransferSyntax &orits = fmi.GetDataSetTransferSyntax();
    if( orits != gdcm::TransferSyntax::ExplicitVRLittleEndian
      && orits != gdcm::TransferSyntax::ImplicitVRLittleEndian
      && orits != gdcm::TransferSyntax::DeflatedExplicitVRLittleEndian )
      {
      std::cerr << "Sorry input Transfer Syntax not supported for this conversion: " << orits << std::endl;
      return 1;
      }

    gdcm::TransferSyntax ts = gdcm::TransferSyntax::ImplicitVRLittleEndian;
    if( explicitts )
      {
      ts = gdcm::TransferSyntax::ExplicitVRLittleEndian;
      }
    else if( deflated )
      {
      ts = gdcm::TransferSyntax::DeflatedExplicitVRLittleEndian;
      }
    std::string tsuid = gdcm::TransferSyntax::GetTSString( ts );
    if( tsuid.size() % 2 == 1 )
      {
      tsuid.push_back( 0 ); // 0 padding
      }
    gdcm::DataElement de( gdcm::Tag(0x0002,0x0010) );
    de.SetByteValue( tsuid.data(), (uint32_t)tsuid.size() );
    de.SetVR( gdcm::Attribute<0x0002, 0x0010>::GetVR() );
    fmi.Clear();
    fmi.Replace( de );

    fmi.SetDataSetTransferSyntax(ts);

    if( explicitts || deflated )
      {
      gdcm::FileExplicitFilter fef;
      fef.SetChangePrivateTags( (changeprivatetags > 0 ? true: false));
      fef.SetFile( reader.GetFile() );
      if( !fef.Change() )
        {
        std::cerr << "Failed to change: " << filename << std::endl;
        return 1;
        }
      }

    if( !writer.Write() )
      {
      std::cerr << "Failed to write: " << outfilename << std::endl;
      return 1;
      }

    return 0;

}

} // end anonymous namespace

// Helper function to create error result
static gdcm_result_t create_error_result(const char* message) {
    gdcm_result_t result;
    result.success = 0;
    strncpy(result.error_message, message, sizeof(result.error_message) - 1);
    result.error_message[sizeof(result.error_message) - 1] = '\0';
    return result;
}

// Helper function to create success result
static gdcm_result_t create_success_result() {
    gdcm_result_t result;
    result.success = 1;
    result.error_message[0] = '\0';
    return result;
}

extern "C" gdcm_result_t gdcm_convert_image(const char* input_file, const char* output_file,
                                           compression_type_t compression, double quality_or_rate) {
    try {
        gdcm::PixmapReader reader;
        reader.SetFileName(input_file);
        if (!reader.Read()) {
            return create_error_result("Could not read input file");
        }

        gdcm::Pixmap &image = reader.GetPixmap();
        gdcm::ImageChangeTransferSyntax change;
        change.SetForce(false);
        change.SetCompressIconImage(false);

        // Not really the right info, but we're trying to create an output file identical to the gdcmconv command
        // so that we can compare output file hashes for sanity checks.
        gdcm::FileMetaInformation::SetImplementationClassUID("1.2.826.0.1.3680043.2.1143.107.104.103.115.3.0.24");
        gdcm::FileMetaInformation::SetImplementationVersionName("GDCM 3.0.24");
        gdcm::FileMetaInformation::SetSourceApplicationEntityTitle("gdcmconv");

        // Configure compression based on type
        switch (compression) {
            case COMPRESSION_RAW: {
                const gdcm::TransferSyntax &ts = image.GetTransferSyntax();
                if (ts.IsExplicit()) {
                    change.SetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);
                } else {
                    change.SetTransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian);
                }
                break;
            }
            case COMPRESSION_JPEG_LOSSY: {
                const gdcm::PixelFormat &pf = image.GetPixelFormat();
                gdcm::JPEGCodec jpegcodec;
                jpegcodec.SetLossless(false);
                if (quality_or_rate > 0) {
                    jpegcodec.SetQuality(quality_or_rate);
                }
                if (pf.GetBitsAllocated() > 8) {
                    change.SetTransferSyntax(gdcm::TransferSyntax::JPEGExtendedProcess2_4);
                } else {
                    change.SetTransferSyntax(gdcm::TransferSyntax::JPEGBaselineProcess1);
                }
                change.SetUserCodec(&jpegcodec);
                break;
            }
            case COMPRESSION_JPEG_LOSSLESS:
                change.SetTransferSyntax(gdcm::TransferSyntax::JPEGLosslessProcess14_1);
                break;
            case COMPRESSION_JPEG2000_LOSSY: {
                gdcm::JPEG2000Codec j2kcodec;
                if (quality_or_rate > 0) {
                    j2kcodec.SetQuality(0, quality_or_rate);
                }
                j2kcodec.SetReversible(false);
                change.SetTransferSyntax(gdcm::TransferSyntax::JPEG2000);
                change.SetUserCodec(&j2kcodec);
                break;
            }
            case COMPRESSION_JPEG2000_LOSSLESS:
                change.SetTransferSyntax(gdcm::TransferSyntax::JPEG2000Lossless);
                break;
            case COMPRESSION_JPEGLS_LOSSY: {
                gdcm::JPEGLSCodec jpeglscodec;
                jpeglscodec.SetLossless(false);
                if (quality_or_rate > 0) {
                    jpeglscodec.SetLossyError(static_cast<int>(quality_or_rate));
                }
                change.SetTransferSyntax(gdcm::TransferSyntax::JPEGLSNearLossless);
                change.SetUserCodec(&jpeglscodec);
                break;
            }
            case COMPRESSION_JPEGLS_LOSSLESS:
                change.SetTransferSyntax(gdcm::TransferSyntax::JPEGLSLossless);
                break;
            case COMPRESSION_RLE:
                change.SetTransferSyntax(gdcm::TransferSyntax::RLELossless);
                break;
            case COMPRESSION_DEFLATED:
                change.SetTransferSyntax(gdcm::TransferSyntax::DeflatedExplicitVRLittleEndian);
                break;
            default:
                return create_error_result("Unsupported compression type");
        }

        change.SetInput(image);
        if (!change.Change()) {
            return create_error_result("Could not change transfer syntax");
        }

        gdcm::PixmapWriter writer;
        writer.SetFileName(output_file);
        writer.SetFile(reader.GetFile());
        writer.SetPixmap(change.PixmapToPixmapFilter::GetOutput());

        if (!writer.Write()) {
            return create_error_result("Failed to write output file");
        }

        return create_success_result();
    } catch (const std::exception& e) {
        return create_error_result(e.what());
    } catch (...) {
        return create_error_result("Unknown error occurred");
    }
}

extern "C" gdcm_result_t gdcm_convert_transfer_syntax(const char* input_file, const char* output_file,
                                                     int explicit_ts, int implicit_ts) {
    try {
        return change_transfersyntax(std::string(input_file), std::string(output_file),
                                   explicit_ts, implicit_ts, 0, 0, 0) == 0
               ? create_success_result()
               : create_error_result("Failed to change transfer syntax");
    } catch (const std::exception& e) {
        return create_error_result(e.what());
    } catch (...) {
        return create_error_result("Unknown error occurred");
    }
}

extern "C" gdcm_result_t gdcm_apply_lut(const char* input_file, const char* output_file, int rgb8) {
    try {
        gdcm::PixmapReader reader;
        reader.SetFileName(input_file);
        if (!reader.Read()) {
            return create_error_result("Could not read input file");
        }

        const gdcm::Pixmap &image = reader.GetPixmap();
        gdcm::ImageApplyLookupTable lutfilt;
        lutfilt.SetInput(image);
        lutfilt.SetRGB8(rgb8 != 0);

        if (!lutfilt.Apply()) {
            return create_error_result("Could not apply LUT");
        }

        gdcm::PixmapWriter writer;
        writer.SetFileName(output_file);
        writer.SetFile(reader.GetFile());
        writer.SetPixmap(lutfilt.PixmapToPixmapFilter::GetOutput());

        if (!writer.Write()) {
            return create_error_result("Failed to write output file");
        }

        return create_success_result();
    } catch (const std::exception& e) {
        return create_error_result(e.what());
    } catch (...) {
        return create_error_result("Unknown error occurred");
    }
}

extern "C" gdcm_result_t gdcm_remove_tags(const char* input_file, const char* output_file,
                                         int remove_private, int remove_retired, int remove_group_length) {
    try {
        gdcm::Reader reader;
        reader.SetFileName(input_file);
        if (!reader.Read()) {
            return create_error_result("Could not read input file");
        }

        gdcm::MediaStorage ms;
        ms.SetFromFile(reader.GetFile());
        if (ms == gdcm::MediaStorage::MediaStorageDirectoryStorage) {
            return create_error_result("DICOMDIR is not supported");
        }

        gdcm::Anonymizer ano;
        ano.SetFile(reader.GetFile());

        if (remove_group_length && !ano.RemoveGroupLength()) {
            return create_error_result("Could not remove group length");
        }

        if (remove_retired && !ano.RemoveRetired()) {
            return create_error_result("Could not remove retired tags");
        }

        if (remove_private && !ano.RemovePrivateTags()) {
            return create_error_result("Could not remove private tags");
        }

        gdcm::Writer writer;
        writer.SetFileName(output_file);
        writer.SetFile(ano.GetFile());

        if (!writer.Write()) {
            return create_error_result("Failed to write output file");
        }

        return create_success_result();
    } catch (const std::exception& e) {
        return create_error_result(e.what());
    } catch (...) {
        return create_error_result("Unknown error occurred");
    }
}
