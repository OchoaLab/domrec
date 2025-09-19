# a pure R version of domrec, easy to understand, but inefficient for very large files
library(genio)

# get arguments
args <- commandArgs( trailingOnly = TRUE )
base_in <- args[1]
base_out <- args[2]
mode <- args[3]

# skip regular checks, this is for internal use only

# load original data
data <- read_plink( base_in, verbose = FALSE )
X <- data$X

# apply desired transformation
if ( mode == 'dev' ) {
    X[ X == 2 ] <- 0
    X[ X == 1 ] <- 2
} else {
    # for dom and rec, need to know when we don't have minor alleles
    afs <- rowMeans( X, na.rm = TRUE )/2
    minor <- afs <= 0.5

    # make sure we have both cases (assumed otherwise), or weird things could happen
    stopifnot( !all( minor ) )
    stopifnot( !all( !minor ) )
    
    X_minor <- X[ minor, ]
    X_major <- X[ !minor, ]
    
    if ( mode == 'dom' ) {
        X_minor[ X_minor == 1 ] <- 2
        X_major[ X_major == 1 ] <- 0 # reverse pattern
    } else if ( mode == 'rec' ) {
        X_minor[ X_minor == 1 ] <- 0
        X_major[ X_major == 1 ] <- 2
    }

    # overwrite data back
    X[ minor, ] <- X_minor
    X[ !minor, ] <- X_major
}

# save edited genotype data to output
write_bed( base_out, X, verbose = FALSE )
