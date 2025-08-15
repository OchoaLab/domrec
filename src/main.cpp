#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include "constants.h"
#include "count_lines.h"

// dominant or recessive is for the counted/effect variant, but it is reverse for the other variant!

int main(int argc, char **argv) {
  // 4 because it counts its own name as argument always
  if ( argc != 4 ) {
    std::cout << "Usage: admixcor <base_in> <base_out> <mode>\nBase is prefix of .bed files (without extension), assumes .bim and .fam also exist\nModes of output encoding: dom (dominance), rec (recessive), dev (dominance deviation)\n";
    return EXIT_FAILURE;
  }

  // separate arguments
  std::string base_in = argv[1];
  std::string base_out = argv[2];
  std::string mode = argv[3];

  // make sure mode is valid
  if ( mode != "dom" && mode != "rec" && mode != "dev" ) {
    std::cerr << "Error: third argument <mode> must equal 'dom', 'rec', or 'dev' only\n";
    return EXIT_FAILURE;
  }

  // add file extensions
  std::string file_in = base_in + ".bed";
  std::string file_out = base_out + ".bed";

  // make sure input exists
  if ( ! std::filesystem::exists( file_in ) ) {
    std::cerr << "Error: input file doesn't exist: " << file_in << "\n";
    return EXIT_FAILURE;
  }
  // and conversely, output should not exist
  if ( std::filesystem::exists( file_out ) ) {
    std::cerr << "Error: output file exists (will not overwrite): " << file_out << "\n";
    return EXIT_FAILURE;
  }

  // read bim and fam files to get dimensions only
  size_t n_ind = count_lines( base_in + ".fam" );
  size_t m_loci = count_lines( base_in + ".bim" );
  // std::cout << "Dims: " << m_loci << " x " << n_ind << "\n";
  
  ////////////////
  // OPEN INPUT //
  ////////////////

  // open input file in "binary" mode
  std::ifstream file_in_stream( file_in, std::ios::binary );
  if ( !file_in_stream.is_open() ) {
    std::cerr << "Error: could not open BED file for reading: " << file_in << "\n";
    return EXIT_FAILURE;
  }

  /////////////////////////////////////
  // OPEN INPUT: check magic numbers //
  /////////////////////////////////////

  // number of columns (bytes) in input (for buffer), after byte compression
  // size set for full row, but overloaded used first for this header comparison
  // chose size_t to have it match n_buf_read value returned by fread
  size_t n_buf = ( n_ind + 3 ) / 4;
  // initialize row buffer
  // NOTE: if n_buf is less than 3, as it does in a toy unit test, insist on at least 3 for input, because that's how big the header is!
  std::vector<char> buffer_in( n_buf > 3 ? n_buf : 3 );
  std::vector<char> buffer_out( n_buf );

  // read header bytes (magic numbers)
  if ( ! file_in_stream.read( buffer_in.data(), 3 ) ) {
    std::cerr << "Error: input BED file did not have a complete header (3-byte magic numbers)!\n";
    return EXIT_FAILURE;
  }
  
  // require that they match our only supported specification of locus-major order and latest format
  size_t pos;
  for (pos = 0; pos < 3; pos++) {
    if ( plink_bed_byte_header[pos] != buffer_in[pos] ) {
      std::cerr << "Error: input BED file is not in supported format.  Either magic numbers do not match, or requested sample-major format is not supported.  Only latest locus-major format is supported!\n";
      return EXIT_FAILURE;
    }
  }
  
  /////////////////
  // OPEN OUTPUT //
  /////////////////

  // open output file
  std::ofstream file_out_stream( file_out, std::ios::binary );
  if ( !file_out_stream.is_open() ) {
    std::cerr << "Could not open BED file for writing: " << file_out << "\n";
    return EXIT_FAILURE;
  }
  
  // write header
  // assume standard locus-major order and latest format
  file_out_stream.write( (char *)plink_bed_byte_header, 3 );
  
  //////////////////////////////
  // read and write genotypes //
  //////////////////////////////
  
  // navigate data and process
  size_t i, j, k, rem;
  unsigned char buf_in_k; // working of buffer at k'th position
  unsigned char xij; // copy of extracted genotype
  for (i = 0; i < m_loci; i++) {
    
    // read whole row into buffer
    if ( ! file_in_stream.read( buffer_in.data(), n_buf ) ) {
      std::cerr << "Error: truncated file: row " << i+1 << " terminated at " << file_in_stream.gcount() << " bytes, expected " << n_buf << ".\n"; // convert to 1-based coordinates
      return EXIT_FAILURE;
    }
    
    // zero out output buffer for new row
    std::fill( buffer_out.begin(), buffer_out.end(), 0 );

    // process buffer now!

    // always reset these at start of row
    j = 0; // individuals
    rem = 0; // to map bit position within byte

    // navigate buffer positions k (not individuals j)
    for (k = 0; k < n_buf; k++) {
      
      // copy down this value, which will be getting edited
      buf_in_k = buffer_in[k];
      
      // navigate the four positions
      // pos is just a dummy counter not really used except to know when to stop
      // update j too, accordingly
      for (pos = 0; pos < 4; pos++, j++) {

	if (j < n_ind) {
	  // extract current genotype using this mask
	  // (3 == 00000011 in binary)
	  xij = buf_in_k & 3;

	  // REENCODE! //

	  // |   ORIGINAL  |   NEW (paper)   |
	  // | paper | BED | dev | dom | rec |
	  // | 0     | 3   | 0   | 0   | 0   |
	  // | 1     | 2   | 2   | 2   | 0   |
	  // | 2     | 0   | 0   | 2   | 2   |
	  // | NA    | 1   | NA  | NA  | NA  |

	  // note that 0 and NA are always fixed
	  // all cases require editing the heterozygote
	  if ( xij == 2 ) {
	    // in paper encoding, this maps 1 to 0 (rec) or 2 (dom and dev)
	    xij = mode == "rec" ? 3 : 0;
	  } else if ( xij == 0 && mode == "dev" )
	      xij = 3; // in paper encoding, this maps 2 to 0 (dev only)
	  
	  // shift input packed data, throwing away genotype we just processed
	  buf_in_k = buf_in_k >> 2;
	  // and push to output
	  buffer_out[k] |= (xij << rem);

	  // update these variables for next round
	  if (rem == 6) {
	    rem = 0; // start a new round
	  } else {
	    rem += 2; // increment as usual (always factors of two)
	  }
	}
      }
      // finished byte
      
    }
    // finished row

    // write buffer (row) out 
    file_out_stream.write( buffer_out.data(), n_buf );
  }
  // finished matrix/file!

  /////////////////
  // CLOSE FILES //
  /////////////////

  // let's check that file was indeed done
  // apparently we just have to try to read more, and test that it didn't
  if ( file_in_stream.read( buffer_in.data(), 1 ) ) {
    std::cerr << "Error: input BED file continued unexpectedly!  Either the specified dimensions are incorrect or the input file is corrupt!\n";
    return EXIT_FAILURE;
  }
  file_in_stream.close();
  
  file_out_stream.close();
  
  return EXIT_SUCCESS;
}
